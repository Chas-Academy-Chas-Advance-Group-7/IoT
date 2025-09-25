#include "tasks/networkstatus_task.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>

extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t networkEventMutex;

#define NETWORK_CONNECTED_BIT BIT0

void networkStatusTask(void *pvParameters)
{
    while (1)
    {
        safePrintf("Network status task started\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
