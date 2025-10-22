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
#define BACKEND_URL "https://your-server.com/api/data"

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
const char digicertG2[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
MrY=
-----END CERTIFICATE-----
)EOF";

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
                http.addHeader("X-API-Key", "Test_key"); // Add your API key here

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
