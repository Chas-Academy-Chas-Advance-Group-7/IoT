#include "tasks/backend_task.h"
#include "sensor_data.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;
extern QueueHandle_t networkQueue;

void backendTask(void *parameter)
{
    safePrintf("Backend task started - waiting for processed sensor data\n");

    sensor_message_t receivedData;
    processed_data_t processedData;

    while (1)
    {
        if (xQueueReceive(dataQueue, &receivedData, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            safePrintf("Processing sensor data from %s\n", receivedData.device_id);

            if (receivedData.is_valid)
            {
                snprintf(processedData.json, sizeof(processedData.json),
                         "{"
                         "\"device_id\":\"%s\","
                         "\"sensor_type\":%d,"
                         "\"sensor_name\":\"%s\","
                         "\"value\":%.2f,"
                         "\"timestamp\":%lu,"
                         "\"rssi\":%d"
                         "}",
                         receivedData.device_id, receivedData.sensor_type, receivedData.sensor_name,
                         receivedData.value, receivedData.timestamp, receivedData.rssi);

                safePrintf("Generated JSON: %s\n", processedData.json);

                if (xQueueSend(networkQueue, &processedData, pdMS_TO_TICKS(100)) != pdTRUE)
                {
                    safePrintf("Failed to queue processed data\n");
                }
                else
                {
                    safePrintf("Data queued for backend transmission\n");
                }
            }
            else
            {
                safePrintf("Received invalid sensor data from %s\n", receivedData.device_id);
            }
        }
    }
}
