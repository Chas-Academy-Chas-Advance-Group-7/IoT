#include "freertos/projdefs.h"
#include "sensor_data.h"
#include "tasks/backend_task.h"
#include "tasks/broker_task.h"
#include "tasks/networkstatus_task.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>

QueueHandle_t dataQueue;
QueueHandle_t networkQueue;

EventGroupHandle_t networkEventGroup;
#define SYSTEM_READY_BIT BIT1

SemaphoreHandle_t serialMutex;
SemaphoreHandle_t networkEventMutex;

void setup()
{
    Serial.begin(115200);

    serialMutex = xSemaphoreCreateMutex();
    networkEventMutex = xSemaphoreCreateMutex();

    if (serialMutex == NULL)
    {
        safePrintf("Failed to create serial mutex\n");
        while (1)
            ;
    }

    networkEventGroup = xEventGroupCreate();

    dataQueue = xQueueCreate(10, sizeof(sensor_message_t));
    networkQueue = xQueueCreate(10, sizeof(processed_data_t));

    // measure needed stack size at a later time
    // Most likely bluetooth task
    xTaskCreate(backendTask, "Communication Task", 8192, NULL, 1, NULL);
    // Process and create payloads
    xTaskCreate(brokerTask, "Processing Task", 8192, NULL, 1, NULL);
    // Handle network connection (pin to core?)
    xTaskCreate(networkStatusTask, "Network Status Task", 8192, NULL, 1, NULL);

    if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        xEventGroupSetBits(networkEventGroup, SYSTEM_READY_BIT);
        xSemaphoreGive(networkEventMutex);
    }
    else
    {
        safePrint("Failed to initialize system ready bit\n");
        while (1)
            ;
    }

    safePrintf("System booted\n");
}

void loop() {}
