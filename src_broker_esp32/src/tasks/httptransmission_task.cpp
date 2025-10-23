/**
 * @file httptransmission_task.cpp
 * @brief FreeRTOS task for transmitting aggregated sensor data to a backend server via HTTP/HTTPS.
 *
 * This task waits for processed (aggregated) JSON data from the `networkQueue`,
 * checks for a valid Wi-Fi connection, and transmits the data using HTTP POST requests.
 * It supports multiple retry attempts and re-queues data if transmission fails.
 *
 * Thread-safe logging is handled using `safePrintf()`.
 *
 * ### Workflow
 * 1. Wait for Wi-Fi connection using `networkEventGroup`.
 * 2. Receive `processed_data_t` from `networkQueue`.
 * 3. Send data via HTTP POST to `BACKEND_URL`.
 * 4. Retry on failure up to `MAX_HTTP_RETRIES`.
 * 5. Re-queue message if all retries fail.
 *
 * @note This task runs indefinitely.
 */

#include "tasks/httptransmission_task.h"
#include "sensor_data.h"
#include "utils/threadsafe_serial.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

/// Event group used to track Wi-Fi connection state.
extern EventGroupHandle_t networkEventGroup;

/// Queue containing aggregated JSON data ready for transmission.
extern QueueHandle_t networkQueue;

/// Bit in `networkEventGroup` indicating Wi-Fi connection.
#define NETWORK_CONNECTED_BIT BIT0

/// Backend URL to which JSON payloads are sent.
#define BACKEND_URL "https://anabel-unconnived-subcreatively.ngrok-free.dev"

/// Maximum number of retry attempts for failed transmissions.
#define MAX_HTTP_RETRIES 3

/// Delay between retry attempts (in milliseconds).
#define RETRY_DELAY_MS 3000

/**
 * @brief DigiCert Global Root G2 certificate for HTTPS connections.
 *
 * This certificate is required for establishing a secure TLS connection
 * with servers signed by DigiCert.
 */
const char digicertG2[] PROGMEM =
    "-----BEGIN CERTIFICATE-----\n"
    "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\n"
    "-----END CERTIFICATE-----\n";

/**
 * @brief FreeRTOS task responsible for sending JSON data to a backend server.
 *
 * This function blocks until Wi-Fi is connected, then continuously waits
 * for JSON data in `networkQueue`. When data is received, it performs an
 * HTTP POST request to the backend server. If the transmission fails,
 * it retries a number of times defined by `MAX_HTTP_RETRIES`, with a delay
 * between attempts. If all retries fail, the data is re-queued for later
 * transmission.
 *
 * @param pvParameters Unused task parameter (required by FreeRTOS).
 *
 * @code
 * // Example task creation:
 * xTaskCreate(httpTransmissionTask, "HTTPTask", 8192, NULL, 1, NULL);
 * @endcode
 */
void httpTransmissionTask(void *pvParameters)
{
    safePrintf("[HTTP] Transmission task started.\n");

    esp_log_level_set("ssl_client", ESP_LOG_DEBUG);

    processed_data_t transferData;

    // Initialize secure Wi-Fi client
    static WiFiClientSecure client;
    client.setCACert(digicertG2);

    static HTTPClient http;

    while (true)
    {
        // Wait for Wi-Fi connection
        EventBits_t bits = xEventGroupWaitBits(networkEventGroup, NETWORK_CONNECTED_BIT, pdFALSE,
                                               pdTRUE, portMAX_DELAY);

        if ((bits & NETWORK_CONNECTED_BIT) == 0)
        {
            safePrintf("[HTTP] Waiting for Wi-Fi connection...\n");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        // Wait indefinitely for data from the backend task
        if (xQueueReceive(networkQueue, &transferData, portMAX_DELAY) == pdTRUE)
        {
            safePrintf("[HTTP] Preparing to send data:\n%s\n", transferData.json);

            bool success = false;

            // Retry loop
            for (int attempt = 1; attempt <= MAX_HTTP_RETRIES; ++attempt)
            {
                safePrintf("[HTTP] Attempt %d to send data...\n", attempt);

                http.begin(client, BACKEND_URL);
                http.addHeader("Content-Type", "application/json");
                http.addHeader("X-API-Key", "Test_key");         // Add your API key here
                http.addHeader("User-Agent", "ESP32Client/1.0"); // 👈 Add this line here

                int httpResponseCode = http.POST(String(transferData.json));

                if (httpResponseCode > 0)
                {
                    safePrintf("[HTTP] POST successful. Response code: %d\n", httpResponseCode);
                    String response = http.getString();
                    safePrintf("[HTTP] Server response: %s\n", response.c_str());
                    success = true;
                    http.end();
                    break;
                }
                else
                {
                    safePrintf("[HTTP] Attempt %d failed. Error code: %d\n", attempt,
                               httpResponseCode);
                    http.end();

                    if (attempt < MAX_HTTP_RETRIES)
                    {
                        safePrintf("[HTTP] Retrying in %d ms...\n", RETRY_DELAY_MS);
                        vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
                    }
                }
            }

            // Re-queue data if all attempts failed
            if (!success)
            {
                safePrintf("[HTTP] Transmission failed after %d attempts. Re-queueing data.\n",
                           MAX_HTTP_RETRIES);

                if (xQueueSendToFront(networkQueue, &transferData, pdMS_TO_TICKS(100)) != pdTRUE)
                {
                    safePrintf("[HTTP] Failed to re-queue data. Message may be lost.\n");
                }
            }
        }
    }
}
