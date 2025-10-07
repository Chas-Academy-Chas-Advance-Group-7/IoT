#include "tasks/networkstatus_task.h"
#include "sensor_data.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>

extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t networkEventMutex;
extern QueueHandle_t networkQueue;

#define NETWORK_CONNECTED_BIT BIT0

void networkStatusTask(void *pvParameters)
{
    safePrintf("Network status task started - monitoring system health\n");

    processed_data_t processedData;
    uint32_t lastDataReceived = millis();
    uint32_t dataCount = 0;

    while (1)
    {
        if (xQueueReceive(networkQueue, &processedData, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            lastDataReceived = millis();
            dataCount++;

            safePrintf("=== Network Queue Data ===\n");
            safePrintf("Received JSON payload #%lu:\n", dataCount);
            safePrintf("%s\n", processedData.json);
            safePrintf("========================\n");
        }
        else
        {
            uint32_t timeSinceLastData = millis() - lastDataReceived;

            if (timeSinceLastData > 30000)
            {
                safePrintf("WARNING: No sensor data received for %lu seconds\n",
                           timeSinceLastData / 1000);
            }

            if (dataCount == 0)
            {
                safePrintf("System waiting for sensor data...\n");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
