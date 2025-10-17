/**
 * @file httptransmission_task.h
 * @brief Header for the HTTP transmission FreeRTOS task.
 *
 * Declares the `httpTransmissionTask` function which continuously waits for
 * processed sensor data on `networkQueue` and sends it to a backend server
 * via HTTP POST requests. Handles retries and re-queues failed transmissions.
 *
 * Example usage:
 * @code
 * #include "tasks/httptransmission_task.h"
 *
 * void setup() {
 *     // Initialize FreeRTOS queues and network event group
 *     // ...
 *
 *     // Create the HTTP transmission task
 *     xTaskCreate(httpTransmissionTask, "HTTPTask", 8192, NULL, 1, NULL);
 * }
 * @endcode
 */

#ifndef HTTPTRANSMISSION_TASK_H
#define HTTPTRANSMISSION_TASK_H

#include <Arduino.h>

/**
 * @brief Task for sending processed sensor data to a backend server via HTTP.
 *
 * Waits for data from `networkQueue`, ensures Wi-Fi connectivity, and sends JSON
 * payloads to `BACKEND_URL`. Retries failed transmissions up to `MAX_RETRIES`
 * and re-queues data if all attempts fail.
 *
 * @param pvParameters Unused task parameter (required by FreeRTOS).
 *
 * @code
 * // Example of task creation
 * xTaskCreate(httpTransmissionTask, "HTTPTask", 8192, NULL, 1, NULL);
 * @endcode
 */
void httpTransmissionTask(void *pvParameters);

#endif // HTTPTRANSMISSION_TASK_H
