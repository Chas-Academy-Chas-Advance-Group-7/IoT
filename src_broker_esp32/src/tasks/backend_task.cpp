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

/** Access key for API authentication */
const char ACCESS_KEY[] = "your_access_key_here";

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

    static processed_data_t processedData;
    const int truckId = 7;
    const int MAX_SENSORS = 10;

    static SensorPacket latestSensors[MAX_SENSORS];
    static int sensorIds[MAX_SENSORS];
    static bool sensorUpdated[MAX_SENSORS];
    static TickType_t lastUpdateTick[MAX_SENSORS]; // NEW: track last packet time

    static bool initialized = false;
    if (!initialized)
    {
        for (int i = 0; i < MAX_SENSORS; i++)
        {
            sensorIds[i] = -1;
            sensorUpdated[i] = false;
            lastUpdateTick[i] = 0;
        }
        initialized = true;
    }

    static StaticJsonDocument<6144> doc;

    static TickType_t aggregationStartTick = 0;
    const TickType_t TIMEOUT_MS = pdMS_TO_TICKS(60000);         // 60s aggregation timeout
    const TickType_t STALE_THRESHOLD_MS = pdMS_TO_TICKS(60000); // 60s stale sensor threshold

    while (true)
    {
        SensorPacket packet;

        // --- Receive sensor packets ---
        if (xQueueReceive(dataQueue, &packet, pdMS_TO_TICKS(500)) == pdTRUE)
        {
            int slot = -1;
            for (int i = 0; i < MAX_SENSORS; i++)
            {
                if (sensorIds[i] == packet.sensor_id)
                {
                    slot = i;
                    break;
                }
                else if (sensorIds[i] == -1 && slot == -1)
                {
                    slot = i;
                }
            }

            if (slot != -1)
            {
                latestSensors[slot] = packet;
                sensorIds[slot] = packet.sensor_id;
                sensorUpdated[slot] = true;
                lastUpdateTick[slot] = xTaskGetTickCount();
                safePrintf("Stored packet for sensor ID %d in slot %d\n", packet.sensor_id, slot);
            }
            else
            {
                safePrintf("No free slot for sensor ID %d - packet dropped\n", packet.sensor_id);
            }
        }

        // --- Start aggregation timer if not started ---
        if (aggregationStartTick == 0)
            aggregationStartTick = xTaskGetTickCount();

        // --- Check active sensors ---
        bool allUpdated = true;
        bool hasData = false;
        int activeSensors = 0;

        for (int i = 0; i < MAX_SENSORS; i++)
        {
            if (sensorIds[i] != -1)
            {
                hasData = true;
                activeSensors++;
                if (!sensorUpdated[i])
                    allUpdated = false;
            }
        }

        // --- Send JSON if all active sensors updated OR timeout ---
        if (hasData && (allUpdated || (xTaskGetTickCount() - aggregationStartTick) >= TIMEOUT_MS))
        {
            aggregationStartTick = 0; // reset timer

            doc.clear();
            doc["access_key"] = ACCESS_KEY;
            doc["truck_id"] = truckId;

            JsonArray sensors = doc["sensors"].to<JsonArray>();

            for (int i = 0; i < MAX_SENSORS; i++)
            {
                if (sensorIds[i] == -1)
                    continue;

                JsonObject sensor = sensors.add<JsonObject>();
                sensor["sensor_id"] = sensorIds[i];
                JsonObject data = sensor["data"].to<JsonObject>();
                // data["timestamp"] = latestSensors[i].sensor_timestamp;
                data["temperature"] = latestSensors[i].temperature;
                data["humidity"] = latestSensors[i].humidity;

                // --- Check for stale sensors ---
                TickType_t age = xTaskGetTickCount() - lastUpdateTick[i];
                if (age >= STALE_THRESHOLD_MS)
                {
                    safePrintf("Warning: Sensor ID %d has stale data (%lu ms old)\n", sensorIds[i],
                               (unsigned long)pdTICKS_TO_MS(age));
                    sensor["stale"] = true; // mark stale in JSON
                }
                else
                {
                    sensor["stale"] = false;
                }

                sensorUpdated[i] = false; // reset flag
            }

            size_t len = serializeJson(doc, processedData.json, sizeof(processedData.json));
            if (len >= sizeof(processedData.json) - 1)
            {
                safePrintf("Warning: JSON truncated, increase buffer size may help\n");
                processedData.json[sizeof(processedData.json) - 1] = '\0';
                len = sizeof(processedData.json) - 1;
            }

            safePrintf("Serialized JSON length: %zu / %zu bytes\n", len,
                       sizeof(processedData.json));
            safePrintf("Aggregated JSON (active sensors: %d): %s\n", activeSensors,
                       processedData.json);

            if (xQueueSend(networkQueue, &processedData, pdMS_TO_TICKS(100)) != pdTRUE)
                safePrintf("Failed to queue aggregated data\n");
            else
                safePrintf("Aggregated data queued for transmission\n");
        }
    }
}
