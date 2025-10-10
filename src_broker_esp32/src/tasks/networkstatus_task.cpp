#include "tasks/networkstatus_task.h"
#include "WiFi_secrets.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <WiFi.h>

extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t networkEventMutex;

#define NETWORK_CONNECTED_BIT BIT0

void networkStatusTask(void *pvParameters)
{
    safePrintf("Network status task started.\n");

    // Mode and credentials for Wifi connection
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    const int maxRetries = 20;
    int retryCount = 0;

    safePrintf("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED && retryCount < maxRetries)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        safePrint(".");
        retryCount++;
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        safePrintf("\nFailed to connect to WiFi.\n");
    }
    else
    {
        safePrintf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        // Signal system that network is ready
        if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            xEventGroupSetBits(networkEventGroup, NETWORK_CONNECTED_BIT);
            xSemaphoreGive(networkEventMutex);
        }
    }

    while (1)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            safePrintf("Wi-Fi lost! Reconnecting...\n");
            WiFi.reconnect();
        }

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
