/**
 * @file networkstatus_task.h
 * @brief Header for the network status FreeRTOS task.
 *
 * Declares the `networkStatusTask` function which connects the ESP32 to Wi-Fi,
 * monitors the connection status, and updates the `networkEventGroup` for
 * other tasks to know when the network is available.
 *
 * Example usage:
 * @code
 * #include "tasks/networkstatus_task.h"
 *
 * void setup() {
 *     // Initialize FreeRTOS event group and mutex
 *     // ...
 *
 *     // Create the network status task
 *     xTaskCreate(networkStatusTask, "NetworkStatusTask", 4096, NULL, 1, NULL);
 * }
 * @endcode
 */

#ifndef NETWORKSTATUS_TASK_H
#define NETWORKSTATUS_TASK_H

#include <Arduino.h>

/**
 * @brief Task for monitoring Wi-Fi connection and updating network events.
 *
 * Connects to Wi-Fi, sets or clears `NETWORK_CONNECTED_BIT` in the
 * `networkEventGroup`, and continuously monitors the connection to attempt
 * reconnection if Wi-Fi is lost.
 *
 * @param pvParameters Unused task parameter (required by FreeRTOS).
 *
 * @code
 * // Example of task creation
 * xTaskCreate(networkStatusTask, "NetworkStatusTask", 4096, NULL, 1, NULL);
 * @endcode
 */
void networkStatusTask(void *pvParameters);

#endif // NETWORKSTATUS_TASK_H
