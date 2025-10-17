/**
 * @file backend_task.cpp
 * @brief Backend processing task for sensor data.
 *
 * This task receives raw sensor packets from a FreeRTOS queue (`dataQueue`),
 * processes them into JSON format, and forwards the resulting data to another
 * queue (`networkQueue`) for network transmission.
 *
 * Thread-safe Serial output is provided through `safePrintf()` to ensure
 * log messages are not interleaved between tasks.
 *
 * ### Workflow
 * 1. Wait for `SensorPacket` objects in `dataQueue`.
 * 2. Convert packet fields into a JSON document.
 * 3. Serialize JSON into a character buffer (`processedData.json`).
 * 4. Send `processedData` to the `networkQueue` for transmission.
 *
 * @note This task runs indefinitely inside a `while(true)` loop.
 * @see safePrintf()
 * @see dataQueue
 * @see networkQueue
 */

#include "tasks/backend_task.h"
#include "sensor_data.h"
#include "sensor_packet_from_sensor.h"
#include "utils/threadsafe_serial.h"
#include <ArduinoJson.h>

/// Queue handle for incoming sensor packets.
extern QueueHandle_t dataQueue;

/// Queue handle for outgoing processed JSON data.
extern QueueHandle_t networkQueue;

/**
 * @brief Backend task function responsible for processing sensor data.
 *
 * This FreeRTOS task continuously listens on `dataQueue` for incoming
 * `SensorPacket` structures. When a packet is received, it converts the data
 * into JSON format and enqueues it to `networkQueue` for network transmission.
 *
 * The task also logs its progress and any potential issues using the
 * thread-safe Serial functions (`safePrintf()`).
 *
 * @param parameter Unused task parameter (required by FreeRTOS task prototype).
 *
 * @code
 * // Task creation example:
 * xTaskCreate(backendTask, "BackendTask", 4096, NULL, 1, NULL);
 * @endcode
 */
void backendTask(void *parameter)
{
    safePrintf("Backend task started - waiting for sensor packets\n");

    SensorPacket packet;
    processed_data_t processedData;

    while (true)
    {
        // Wait up to 1000 ms for incoming sensor data
        if (xQueueReceive(dataQueue, &packet, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            safePrintf("Processing packet from Sensor ID %u\n", packet.sensor_id);

            // Build a JSON document from the sensor packet
            JsonDocument doc;
            doc["sensor_id"] = packet.sensor_id;
            doc["server_package_id"] = packet.server_package_id;
            doc["timestamp"] = packet.sensor_timestamp;
            doc["temperature"] = packet.temperature;
            doc["humidity"] = packet.humidity;
            doc["sequence_number"] = packet.package_sequence_number;

            // Serialize JSON into a character buffer
            size_t len = serializeJson(doc, processedData.json, sizeof(processedData.json));

            // Check if the output was truncated
            if (len >= sizeof(processedData.json) - 1)
<<<<<<< HEAD
                safePrintf("JSON truncated\n");
            == == == = { safePrintf("⚠️ JSON truncated\n");
            continue; // Skip queuing truncated data
        }
>>>>>>> a07b624b4b044f1ecb3d827d7a0a7ebc259a91a2

        safePrintf("Generated JSON: %s\n", processedData.json);

        // Send processed data to the network queue
        if (xQueueSend(networkQueue, &processedData, pdMS_TO_TICKS(100)) != pdTRUE)
            safePrintf("Failed to queue processed data\n");
        else
            safePrintf("Data queued for backend transmission\n");
    }
}
}
