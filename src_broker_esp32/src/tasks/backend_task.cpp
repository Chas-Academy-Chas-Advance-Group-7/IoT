#include "tasks/backend_task.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>

extern QueueHandle_t networkQueue;

void backendTask(void *pvParameter)
{
    while (1)
    {
        safePrintf("Backend task started\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
