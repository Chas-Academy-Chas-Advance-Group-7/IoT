/**
 * @file networkstatus_task.cpp
 * @brief Task for monitoring Wi-Fi connection status and managing network events.
 *
 * This FreeRTOS task connects the ESP32 to Wi-Fi using user-provided credentials,
 * monitors the connection status, and updates the `networkEventGroup` accordingly.
 * It ensures other tasks (like `httpTransmissionTask`) can wait for network connectivity.
 *
 * Thread-safe logging is provided via `safePrintf()` and `safePrint()`.
 *
 * ### Workflow
 * 1. Connect to Wi-Fi at startup with a maximum retry count.
 * 2. Set or clear `NETWORK_CONNECTED_BIT` in `networkEventGroup` based on connectivity.
 * 3. Continuously monitor Wi-Fi status and reconnect if disconnected.
 *
 * @note This task runs indefinitely inside a `while(true)` loop.
 */

#include "tasks/networkstatus_task.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <WiFi.h>

// Include Wi-Fi credentials with fallback
#if __has_include("WiFi_secrets.h")
#include "WiFi_secrets.h" // User-provided credentials
#else
#include "WiFi_secrets_example.h" // Default fallback
#endif

/// Event group for network connection state.
extern EventGroupHandle_t networkEventGroup;

/// Mutex for thread-safe access to networkEventGroup.
extern SemaphoreHandle_t networkEventMutex;

/// Bit in `networkEventGroup` indicating Wi-Fi connection.
#define NETWORK_CONNECTED_BIT BIT0

/**
 * @brief Task for monitoring Wi-Fi status and updating network events.
 *
 * Connects to Wi-Fi using credentials from `WiFi_secrets.h` or fallback, waits
 * for initial connection, and sets/clears `NETWORK_CONNECTED_BIT` in the
 * `networkEventGroup`. Continuously monitors connection and attempts
 * reconnection if Wi-Fi is lost.
 *
 * @param pvParameters Unused task parameter (required by FreeRTOS).
 *
 * @code
 * // Example task creation:
 * xTaskCreate(networkStatusTask, "NetworkStatusTask", 4096, NULL, 1, NULL);
 * @endcode
 */
void networkStatusTask(void *pvParameters)
{
    safePrintf("Network status task started.\n");

    // Configure Wi-Fi mode and connect
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    const int maxRetries = 20;
    int retryCount = 0;

    safePrintf("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED && retryCount < maxRetries)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        safePrint(".");
        retryCount++;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        safePrintf("\nFailed to connect to WiFi.\n");

        // Clear network bit if initial connection fails
        if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            xEventGroupClearBits(networkEventGroup, NETWORK_CONNECTED_BIT);
            xSemaphoreGive(networkEventMutex);
        }
    }
    else
    {
        safePrintf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());

        // Set network bit after successful connection
        if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            xEventGroupSetBits(networkEventGroup, NETWORK_CONNECTED_BIT);
            xSemaphoreGive(networkEventMutex);
        }
    }

    // Continuous monitoring loop
    bool wasConnected = (WiFi.status() == WL_CONNECTED); // Track previous state

    while (1)
    {
        bool isConnected = (WiFi.status() == WL_CONNECTED); // Current state

        if (!isConnected && wasConnected) // Wi-Fi just got disconnected
        {
            safePrintf("Wi-Fi lost! Reconnecting...\n");

            // Clear the network bit on disconnect
            if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
            {
                xEventGroupClearBits(networkEventGroup, NETWORK_CONNECTED_BIT);
                xSemaphoreGive(networkEventMutex);
            }

            WiFi.reconnect(); // Attempt to reconnect
        }
        else if (isConnected && !wasConnected) // Wi-Fi just reconnected
        {
            safePrintf("Wi-Fi reconnected! IP: %s\n", WiFi.localIP().toString().c_str());

            // Set the network bit on reconnect
            if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
            {
                xEventGroupSetBits(networkEventGroup, NETWORK_CONNECTED_BIT);
                xSemaphoreGive(networkEventMutex);
            }
        }

        wasConnected = isConnected;       // Update previous state
        vTaskDelay(pdMS_TO_TICKS(10000)); // Delay between checks
    }
}
