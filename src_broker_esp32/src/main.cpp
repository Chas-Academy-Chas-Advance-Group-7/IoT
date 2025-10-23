/**
 * @file main.cpp
 * @brief Main entry point for the ESP32 sensor aggregation system.
 *
 * Initializes FreeRTOS queues, event groups, semaphores, and tasks:
 * - Backend Task: processes sensor packets and queues JSON for transmission.
 * - Broker Task: scans BLE sensors and receives sensor data.
 * - Network Status Task: connects to Wi-Fi and monitors connectivity.
 * - HTTP Transmission Task: sends queued JSON sensor data to backend server.
 *
 * Thread-safe serial logging is enabled via `serialMutex` and the `safePrint*` functions.
 *
 * Global objects:
 * - `dataQueue`: Queue for raw sensor packets from broker to backend.
 * - `networkQueue`: Queue for processed JSON data from backend to HTTP task.
 * - `networkEventGroup`: Event group for network connection and system readiness flags.
 * - `serialMutex`: Mutex for safe Serial printing.
 * - `networkEventMutex`: Mutex for safely accessing `networkEventGroup`.
 *
 * Example usage:
 * @code
 * void setup() {
 *     // Initializes all queues, semaphores, and tasks
 *     // Sets SYSTEM_READY_BIT when initialization completes
 * }
 *
 * void loop() {
 *     // Not used in FreeRTOS multitasking setup
 * }
 * @endcode
 */

#include "freertos/projdefs.h"
#include "sensor_data.h"
#include "sensor_packet_from_sensor.h"
#include "tasks/backend_task.h"
#include "tasks/broker_task.h"
#include "tasks/httptransmission_task.h"
#include "tasks/networkstatus_task.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>

/// Queue for raw sensor packets from BLE broker to backend task
QueueHandle_t dataQueue;

/// Queue for processed JSON sensor data from backend to HTTP transmission task
QueueHandle_t networkQueue;

/// Event group for network status and system readiness
EventGroupHandle_t networkEventGroup;

/// Bit indicating that the system is fully initialized and ready
#define SYSTEM_READY_BIT BIT1

/// Mutex for thread-safe Serial printing
SemaphoreHandle_t serialMutex;

/// Mutex for safely accessing networkEventGroup
SemaphoreHandle_t networkEventMutex;

/**
 * @brief Arduino setup function.
 *
 * Initializes Serial, creates mutexes and event groups, initializes queues,
 * and starts FreeRTOS tasks for BLE sensor aggregation and HTTP transmission.
 * Sets the `SYSTEM_READY_BIT` when initialization completes.
 */
void setup()
{
    Serial.begin(115200);

    serialMutex = xSemaphoreCreateMutex();
    networkEventMutex = xSemaphoreCreateMutex();

    if (serialMutex == NULL)
    {
        Serial.println("Failed to create serial mutex");
        while (1)
            ;
    }

    networkEventGroup = xEventGroupCreate();

    dataQueue = xQueueCreate(10, sizeof(SensorPacket));
    networkQueue = xQueueCreate(10, sizeof(processed_data_t));

    // TODO: measure needed stack size for each task

    // Create FreeRTOS tasks
    xTaskCreate(backendTask, "Backend Communication Task", 8192, NULL, 1, NULL);
    xTaskCreate(brokerTask, "BLE Broker Task", 8192, NULL, 1, NULL);
    xTaskCreate(networkStatusTask, "Network Status Task", 3072, NULL, 1, NULL);
    xTaskCreate(httpTransmissionTask, "HTTP Transmission Task", 8192, NULL, 1, NULL);

    // Set SYSTEM_READY_BIT
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

/**
 * @brief Arduino loop function.
 *
 * Empty since the system runs entirely on FreeRTOS tasks.
 */
void loop() {}
