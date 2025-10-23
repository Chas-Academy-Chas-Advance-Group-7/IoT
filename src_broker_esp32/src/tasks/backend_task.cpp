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

    // Make processedData static to prevent stack overflow
    static processed_data_t processedData;

    const int truckId = 7; // your truck ID

    // Storage for latest sensor data
    const int MAX_SENSORS = 10; // adjust based on your system

    // These arrays are large and persistent, so make them static
    static SensorPacket latestSensors[MAX_SENSORS];
    static int sensorIds[MAX_SENSORS]; // tracks which sensor is in which slot

    // Initialize sensorIds to -1 (empty)
    static bool initialized = false;
    if (!initialized)
    {
        for (int i = 0; i < MAX_SENSORS; i++)
            sensorIds[i] = -1;
        initialized = true;
    }

    // Static JSON document reduces stack usage; size increased for larger payloads
    static StaticJsonDocument<6144> doc;

    // Track last aggregation tick
    static TickType_t lastSendTick = 0;

    while (true)
    {
        SensorPacket packet;

        // Non-blocking check for incoming sensor packets
        if (xQueueReceive(dataQueue, &packet, pdMS_TO_TICKS(500)) == pdTRUE)
        {
            // Find existing slot or first free slot
            int slot = -1;
            for (int i = 0; i < MAX_SENSORS; i++)
            {
                if (sensorIds[i] == packet.sensor_id)
                {
                    slot = i; // update existing sensor
                    break;
                }
                else if (sensorIds[i] == -1 && slot == -1)
                {
                    slot = i; // first free slot
                }
            }

            if (slot != -1)
            {
                latestSensors[slot] = packet;
                sensorIds[slot] = packet.sensor_id;
                safePrintf("Stored packet for sensor ID %d in slot %d\n", packet.sensor_id, slot);
            }
            else
            {
                safePrintf("No free slot for sensor ID %d - packet dropped\n", packet.sensor_id);
            }
        }

        // Aggregate and send JSON periodically
        if (xTaskGetTickCount() - lastSendTick >= pdMS_TO_TICKS(2000))
        {
            lastSendTick = xTaskGetTickCount();

            // Check if any sensor has data
            bool hasData = false;
            for (int i = 0; i < MAX_SENSORS; i++)
            {
                if (sensorIds[i] != -1)
                {
                    hasData = true;
                    break;
                }
            }
            if (!hasData)
                continue;

            // Clear the JSON document before reuse
            doc.clear();
            doc["truck_id"] = truckId;

            // Create "sensors" array
            JsonArray sensors = doc.createNestedArray("sensors");

            for (int i = 0; i < MAX_SENSORS; i++)
            {
                if (sensorIds[i] == -1)
                    continue;

                JsonObject sensor = sensors.add<JsonObject>();
                sensor["sensor_id"] = sensorIds[i];

                JsonObject data = sensor.createNestedObject("data");
                data["timestamp"] = latestSensors[i].sensor_timestamp;
                data["temperature"] = latestSensors[i].temperature;
                data["humidity"] = latestSensors[i].humidity;
            }

            // Serialize JSON into processedData
            size_t len = serializeJson(doc, processedData.json, sizeof(processedData.json));

            // Check if JSON was truncated
            if (len >= sizeof(processedData.json) - 1)
            {
                safePrintf("Warning: JSON truncated, increase buffer size may help\n");
                processedData.json[sizeof(processedData.json) - 1] =
                    '\0'; // ensure null termination
                len = sizeof(processedData.json) - 1;
            }

            safePrintf("Serialized JSON length: %zu / %zu bytes\n", len,
                       sizeof(processedData.json));
            safePrintf("Aggregated JSON: %s\n", processedData.json);

            // Send aggregated JSON to network queue
            if (xQueueSend(networkQueue, &processedData, pdMS_TO_TICKS(100)) != pdTRUE)
                safePrintf("Failed to queue aggregated data\n");
            else
                safePrintf("Aggregated data queued for transmission\n");
        }
    }
}
