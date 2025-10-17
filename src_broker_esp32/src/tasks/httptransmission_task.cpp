/**
 * @file httptransmission_task.cpp
 * @brief HTTP transmission task for sending sensor data to a backend server.
 *
 * This FreeRTOS task waits for processed sensor data on `networkQueue` and
 * sends it to a backend server over HTTP/HTTPS. The task handles Wi-Fi
 * connectivity, retries failed transmissions, and can re-queue data if all
 * attempts fail.
 *
 * Thread-safe logging is provided via `safePrintf()`.
 *
 * ### Workflow
 * 1. Wait for Wi-Fi connection using `networkEventGroup`.
 * 2. Receive `processed_data_t` from `networkQueue`.
 * 3. Send data via HTTP POST to `BACKEND_URL`.
 * 4. Retry up to `MAX_RETRIES` on failure.
 * 5. Re-queue data if all retries fail.
 *
 * @note This task runs indefinitely inside a `while(true)` loop.
 */

#include "tasks/httptransmission_task.h"
#include "sensor_data.h"
#include "utils/threadsafe_serial.h"
#include <HTTPClient.h>
#include <WiFi.h>
// #include <WiFiClientSecure.h> // Old secure client

/// Event group for network connection state.
extern EventGroupHandle_t networkEventGroup;

/// Mutex for safe access to network events (currently unused here).
extern SemaphoreHandle_t networkEventMutex;

/// Queue containing data ready for HTTP transmission.
extern QueueHandle_t networkQueue;

/// Bit in `networkEventGroup` indicating Wi-Fi connection.
#define NETWORK_CONNECTED_BIT BIT0

/// Backend URL to which sensor data will be sent.
#define BACKEND_URL "your-URL-here"

/// Maximum number of retry attempts for failed HTTP requests.
#define MAX_RETRIES 3

/// Delay between retry attempts in milliseconds.
#define RETRY_DELAY_MS 3000

/**
 * @brief FreeRTOS task for sending sensor data over HTTP/HTTPS.
 *
 * Waits for processed sensor data from `networkQueue`, ensures Wi-Fi
 * connectivity, and sends JSON payloads to the backend server using
 * HTTP POST requests. Handles retries and re-queues unsent data if all
 * attempts fail.
 *
 * @param pvParameters Unused task parameter (required by FreeRTOS).
 *
 * @code
 * // Task creation example:
 * xTaskCreate(httpTransmissionTask, "HTTPTask", 8192, NULL, 1, NULL);
 * @endcode
 */
void httpTransmissionTask(void *pvParameters)
{
    safePrintf("HTTP Transmission task started.\n");

    processed_data_t transferData;

    // --- secure client setup ---
    static WiFiClientSecure client;
    static HTTPClient http;
    client.setInsecure(); // disables certificate validation

    // HTTP client for testing (non-secure)
    // WiFiClient client;
    // HTTPClient http;

    while (1)
    {
        // Wait for Wi-Fi connection (up to 10s)
        EventBits_t bits = xEventGroupWaitBits(networkEventGroup, NETWORK_CONNECTED_BIT, pdFALSE,
                                               pdTRUE, pdMS_TO_TICKS(10000));

        if ((bits & NETWORK_CONNECTED_BIT) == 0)
        {
            safePrintf("Wi-Fi not connected, waiting...\n");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        // Check if data is ready for transmission
        if (xQueueReceive(networkQueue, &transferData, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            safePrintf("Preparing to send data:\n%s\n", transferData.json);

            bool success = false;

            // Attempt HTTP transmission up to MAX_RETRIES times
            for (int attempt = 1; attempt <= MAX_RETRIES; ++attempt)
            {
                http.begin(client, BACKEND_URL);
                http.addHeader("Content-Type", "application/json");

                int httpResponseCode = http.POST(transferData.json);

                if (httpResponseCode > 0)
                {
                    safePrintf("Data successfully sent on attempt %d. Server response: %d\n",
                               attempt, httpResponseCode);
                    String response = http.getString();
                    safePrintf("Response body: %s\n", response.c_str());
                    success = true;
                    http.end();
                    break;
                }
                else
                {
                    safePrintf("Attempt %d failed. Error: %d\n", attempt, httpResponseCode);
                    http.end();

                    if (attempt < MAX_RETRIES)
                    {
                        safePrintf("Retrying in %d ms...\n", RETRY_DELAY_MS);
                        vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
                    }
                }
            }

            // Re-queue message if all retries fail
            if (!success)
            {
                safePrintf("Failed to send data after %d attempts. Re-queueing message.\n",
                           MAX_RETRIES);
                if (xQueueSendToFront(networkQueue, &transferData, pdMS_TO_TICKS(100)) != pdTRUE)
                {
                    safePrintf("Failed to re-queue message. Data may be lost!\n");
                }
            }
        }
        else
        {
            // No data to send, wait a bit
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}
