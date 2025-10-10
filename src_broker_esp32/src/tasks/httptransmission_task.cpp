#include "tasks/httptransmission_task.h"
#include "sensor_data.h"
#include "utils/threadsafe_serial.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t networkEventMutex;

void httpTransmissionTask(void *pvParameters) {}