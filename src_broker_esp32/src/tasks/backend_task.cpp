#include "tasks/backend_task.h"
#include "sensor_data.h"
#include "sensor_packet_from_sensor.h"
#include "utils/threadsafe_serial.h"
#include <ArduinoJson.h>

extern QueueHandle_t dataQueue;
extern QueueHandle_t networkQueue;

void backendTask(void *parameter)
{
    safePrintf("Backend task started - waiting for sensor packets\n");

    SensorPacket packet;
    processed_data_t processedData;

    while (true)
    {
        if (xQueueReceive(dataQueue, &packet, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            safePrintf("Processing packet from Sensor ID %u\n", packet.sensor_id);

            // Only backend task handles JSON
            DynamicJsonDocument doc(512);
            doc["sensor_id"] = packet.sensor_id;
            doc["server_package_id"] = packet.server_package_id;
            doc["timestamp"] = packet.sensor_timestamp;
            doc["temperature"] = packet.temperature;
            doc["humidity"] = packet.humidity;
            doc["sequence_number"] = packet.package_sequence_number;

            size_t len = serializeJson(doc, processedData.json, sizeof(processedData.json));
            if (len == sizeof(processedData.json))
                safePrintf("⚠️ JSON truncated\n");

            safePrintf("Generated JSON: %s\n", processedData.json);

            if (xQueueSend(networkQueue, &processedData, pdMS_TO_TICKS(100)) != pdTRUE)
                safePrintf("❌ Failed to queue processed data\n");
            else
                safePrintf("✅ Data queued for backend transmission\n");
        }
    }
}
