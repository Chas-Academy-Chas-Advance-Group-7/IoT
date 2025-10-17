/**
 * @file broker_task.h
 * @brief Header for the broker FreeRTOS task handling BLE sensor discovery.
 *
 * Declares the `brokerTask` function, which continuously scans for BLE sensors,
 * connects to them, subscribes to notifications, and forwards incoming
 * `SensorPacket`s to the `dataQueue` for backend processing.
 *
 * Example usage:
 * @code
 * #include "tasks/broker_task.h"
 *
 * void setup() {
 *     // Initialize FreeRTOS queues here
 *     // ...
 *
 *     // Create the broker task
 *     xTaskCreate(brokerTask, "BrokerTask", 8192, NULL, 1, NULL);
 * }
 * @endcode
 */

#ifndef BROKER_TASK_H
#define BROKER_TASK_H

#include <Arduino.h>

/**
 * @brief Broker task for discovering BLE sensors and handling notifications.
 *
 * This FreeRTOS task continuously scans for BLE sensors with the specified
 * service UUID, connects to new devices, subscribes to notifications, and
 * forwards received sensor packets to the `dataQueue`.
 *
 * @param pvParameters Unused task parameter (required by FreeRTOS).
 *
 * @code
 * // Example of task creation
 * xTaskCreate(brokerTask, "BrokerTask", 8192, NULL, 1, NULL);
 * @endcode
 */
void brokerTask(void *pvParameters);

#endif // BROKER_TASK_H
