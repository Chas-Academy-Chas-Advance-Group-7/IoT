#include "tasks/networkstatus_task.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>

extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t networkEventMutex;

#define NETWORK_CONNECTED_BIT BIT0

void networkStatusTask(void *pvParameters)
{
    safePrintf("Network status task started - ready for network management\n");

    while (1)
    {
        // TODO: Add WiFi

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
