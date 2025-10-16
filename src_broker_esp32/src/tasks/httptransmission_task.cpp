#include "tasks/httptransmission_task.h"
#include "sensor_data.h"
#include "utils/threadsafe_serial.h"
#include <HTTPClient.h>
#include <WiFi.h>
// #include <WiFiClientSecure.h> // Old secure client

extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t networkEventMutex;
extern QueueHandle_t networkQueue;

#define NETWORK_CONNECTED_BIT BIT0

#define BACKEND_URL                                                                                \
    "" // Replace with your actual ngrok HTTP
       // URL

#define MAX_RETRIES 3
#define RETRY_DELAY_MS 3000

void httpTransmissionTask(void *pvParameters)
{
    safePrintf("HTTP Transmission task started.\n");

    processed_data_t transferData;

    // --- Old secure client setup (commented out) ---
    // static WiFiClientSecure clientSecure;
    // static HTTPClient http;
    // clientSecure.setInsecure(); // disables certificate validation

    // New plain HTTP client for testing
    WiFiClient client;
    HTTPClient http;

    while (1)
    {
        // Wait for Wi-Fi connection
        EventBits_t bits = xEventGroupWaitBits(networkEventGroup, NETWORK_CONNECTED_BIT, pdFALSE,
                                               pdTRUE, pdMS_TO_TICKS(10000));

        if ((bits & NETWORK_CONNECTED_BIT) == 0)
        {
            safePrintf("Wi-Fi not connected, waiting...\n");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        // Check if there's data ready to send
        if (xQueueReceive(networkQueue, &transferData, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            safePrintf("Preparing to send data:\n%s\n", transferData.json);

            bool success = false;

            for (int attempt = 1; attempt <= MAX_RETRIES; ++attempt)
            {
                http.begin(client, BACKEND_URL); // Plain HTTP client
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
            vTaskDelay(pdMS_TO_TICKS(2000)); // No data, wait a bit
        }
    }
}
