/**
 * @file sensor_package_manager.h
 * @brief Manages sensor data and the assembly of SensorPacket structures.
 *
 * Declares the SensorPacket structure, global sensor variables, and functions
 * for assembling sensor packets and retrieving timestamps.
 *
 * Example usage:
 * @code
 * #include "sensor_package_manager.h"
 *
 * SensorPacket packet = assembleSensorPacket();
 * Serial.printf("Sensor ID: %u, Temp: %.2f, Humidity: %.2f\n",
 *               packet.sensor_id, packet.temperature, packet.humidity);
 * @endcode
 */

#ifndef SENSOR_PACKAGE_MANAGER_H
#define SENSOR_PACKAGE_MANAGER_H

#include <Arduino.h>

/**
 * @brief Structure representing a sensor packet.
 *
 * Packed to ensure no padding between fields. Contains:
 * - Sensor ID
 * - Timestamp
 * - Temperature
 * - Humidity
 */
typedef struct __attribute__((packed))
{
    uint8_t sensor_id;         /**< Unique sensor ID */
    uint32_t sensor_timestamp; /**< Timestamp in milliseconds */
    float temperature;         /**< Temperature in °C */
    float humidity;            /**< Relative humidity in % */
} SensorPacket;

/** Unique ID of this sensor unit */
extern uint8_t sensor_id;

/** Last read temperature value (°C) */
extern float temperature;

/** Last read humidity value (%) */
extern float humidity;

/** Server package ID counter */
extern uint32_t server_package_id;

/** Sequence number for each packet sent */
extern uint16_t package_sequence_number;

/**
 * @brief Assemble a SensorPacket using current sensor readings.
 *
 * Reads available sensors (e.g., DHT) and fills the SensorPacket structure.
 *
 * @return SensorPacket Filled with current sensor data.
 */
SensorPacket assembleSensorPacket();

/**
 * @brief Retrieve the current timestamp for the sensor packet.
 *
 * Intended to provide a timestamp in milliseconds.
 *
 * @return uint32_t Current timestamp (ms).
 */
uint32_t getTimestamp();

#endif // SENSOR_PACKAGE_MANAGER_H
