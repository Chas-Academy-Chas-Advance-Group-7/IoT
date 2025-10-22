/**
 * @file sensor_package_manager.cpp
 * @brief Manages sensor data and assembly of SensorPacket structures.
 *
 * Provides global sensor variables, functions to assemble sensor packets
 * with readings (e.g., from DHT sensor), and a placeholder for timestamp retrieval.
 *
 * Example usage:
 * @code
 * SensorPacket packet = assembleSensorPacket();
 * Serial.printf("Sensor ID: %u, Temperature: %.2f, Humidity: %.2f\n",
 *               packet.sensor_id, packet.temperature, packet.humidity);
 * @endcode
 */

#include "sensor_package_manager.h"
#include "DHT_handler.h"

/** Unique ID of this sensor unit */
uint8_t sensor_id = 0;

/** Last read temperature value (°C) */
float temperature = 0.0;

/** Last read humidity value (%) */
float humidity = 0.0;

/**
 * @brief Assemble a SensorPacket using current sensor data.
 *
 * Reads the DHT sensor and fills a SensorPacket with:
 * - Sensor ID
 * - Timestamp
 * - Temperature
 * - Humidity
 *
 * @return SensorPacket Filled with current sensor data.
 *
 * @code
 * SensorPacket packet = assembleSensorPacket();
 * Serial.printf("Temp: %.2f, Humidity: %.2f\n", packet.temperature, packet.humidity);
 * @endcode
 */
SensorPacket assembleSensorPacket()
{
    SensorPacket packet;

    packet.sensor_id = sensor_id;
    packet.sensor_timestamp = getTimestamp();

    if (!readDHT(packet))
    {
        Serial.println("Failed to read sensor, invalid package!");
        return packet;
    }

    return packet;
}

/**
 * @brief Retrieve the current timestamp for the sensor packet.
 *
 * Currently not implemented; intended to provide a timestamp in milliseconds.
 *
 * @return uint32_t Current timestamp (ms). Placeholder function.
 */
uint32_t getTimestamp() {}
