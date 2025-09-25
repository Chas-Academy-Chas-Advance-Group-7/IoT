#include "tasks/broker_task.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;
extern QueueHandle_t networkQueue;

void brokerTask(void *parameter)
{
    while (1)
    {
        safePrintf("Broker task started\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
