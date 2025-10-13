#include "tasks/httptransmission_task.h"
#include "sensor_data.h"
#include "utils/threadsafe_serial.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t networkEventMutex;

extern QueueHandle_t networkQueue;

#define NETWORK_CONNECTED_BIT BIT0

#define BACKEND_URL ""

void httpTransmissionTask(void *pvParameters)
{
    safePrintf("http transmission task started.\n");

    processed_data_t transferData;

    while (1)
    {
        // Wait for Wi-Fi connection
        EventBits_t bits = xEventGroupWaitBits(networkEventGroup, NETWORK_CONNECTED_BIT,
                                               pdFALSE, // Don't clear bit
                                               pdTRUE,  // Wait for all bits (just one in this case)
                                               pdMS_TO_TICKS(10000));

        if ((bits & NETWORK_CONNECTED_BIT) == 0)
        {
            safePrintf("WiFi not connected, waiting.....\n");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        // Check if there’s data ready to send
        if (xQueueReceive(networkQueue, &transferData, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            safePrintf("Preparing to send data to backend:\n%s\n", transferData.json);

            static WiFiClientSecure client;
            client.setInsecure(); // For testing only (Disables certificate validation)

            static HTTPClient http;
            http.begin(client, BACKEND_URL);
            http.addHeader("Content-Type", "application/json");

            int httpResponseCode = http.POST(transferData.json);

            if (httpResponseCode > 0)
            {
                safePrintf("Data sent! Server response: %d\n", httpResponseCode);
                String response = http.getString();
                safePrintf("Response body: %s\n", response.c_str());
            }

            else
            {
                safePrintf("Failed to send data. Error: %d\n", httpResponseCode);
            }

            http.end();
        }

        else
        {
            // No data in queue
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}