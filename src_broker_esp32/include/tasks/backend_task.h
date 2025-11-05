/**
 * @file backend_task.h
 * @brief Header for the backend FreeRTOS task processing sensor data.
 *
 * Declares the `backendTask` function which continuously receives sensor packets
 * from the `dataQueue`, converts them to JSON, and sends them to the
 * `networkQueue` for transmission.
 *
 * Example usage:
 * @code
 * #include "tasks/backend_task.h"
 *
 * void setup() {
 *     // Initialize queues here
 *     // ...
 *
 *     // Create the backend task
 *     xTaskCreate(backendTask, "BackendTask", 4096, NULL, 1, NULL);
 * }
 * @endcode
 */

#ifndef BACKEND_TASK_H
#define BACKEND_TASK_H

#include <Arduino.h>

/**
 * @brief Backend task for processing sensor data and producing JSON output.
 *
 * This FreeRTOS task waits for incoming `SensorPacket` data on the
 * `dataQueue`, converts it to JSON format, and sends it to `networkQueue`.
 * It also logs progress and potential issues using the thread-safe Serial
 * functions.
 *
 * @param pvParameters Unused task parameter (required by FreeRTOS).
 *
 * @note This task runs indefinitely.
 *
 * @code
 * // Task creation example:
 * xTaskCreate(backendTask, "BackendTask", 4096, NULL, 1, NULL);
 * @endcode
 */
void backendTask(void *pvParameters);

#endif // BACKEND_TASK_H
