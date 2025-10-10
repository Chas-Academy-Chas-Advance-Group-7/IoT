#include "tasks/networkstatus_task.h"
#include "WiFi_secrets.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <WiFi.h>

extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t networkEventMutex;

#define NETWORK_CONNECTED_BIT BIT0

void networkStatusTask(void *pvParameters)
{
    safePrintf("Network status task started.\n");

    // Mode and credentials for Wi-Fi connection
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Initial connection with max retries
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

    // Continuous monitoring and reconnection loop
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
