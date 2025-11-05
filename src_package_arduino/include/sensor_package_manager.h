/**
 * @file sensor_package_manager.h
 * @brief Sensor packet layout and helpers for assembling SensorPacket structs.
 *
 * This header defines the binary layout for sensor packets emitted by the
 * sensor unit and helper functions to create them. The binary layout is
 * consumed by the broker; therefore any change to this structure must be
 * coordinated across projects (update `src_broker_esp32/include/sensor_packet_from_sensor.h`).
 *
 * Notes on binary compatibility:
 * - The `SensorPacket` type is marked `packed` to avoid compiler-inserted
 *   padding. Field order and sizes are part of the public contract.
 * - Endianness: the current fields are single-element primitive types
 *   (uint8_t, float). Keep in mind floats have platform-dependent
 *   representations; the broker expects the same float representation.
 * - When adding fields, update the packet struct on the broker side and
 *   the JSON mapping in the backend task.
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
 * @brief Binary representation of a sensor packet.
 *
 * The struct is intentionally packed to provide a stable, compact
 * representation for BLE notifications. Do not reorder or change types
 * without updating the broker-side parser.
 *
 * Fields:
 * - `sensor_id` : 1 byte identifier for the sensor unit.
 * - `temperature` : 4-byte float (°C). Ensure broker uses compatible
 *   float parsing/ABI.
 * - `humidity` : 4-byte float (% RH).
 *
 * Size: typically 9 bytes on platforms with 32-bit float. Confirm with
 * `sizeof(SensorPacket)` if you change fields.
 */
typedef struct __attribute__((packed))
{
    uint8_t sensor_id; /**< Unique sensor ID */
    // uint32_t sensor_timestamp; /**< Timestamp in milliseconds - reserved */
    float temperature; /**< Temperature in °C */
    float humidity;    /**< Relative humidity in % */
} SensorPacket;

/** Unique ID of this sensor unit (set in configuration or at runtime) */
extern uint8_t sensor_id;

/** Last read temperature value (°C) cached by the sampling routine */
extern float temperature;

/** Last read humidity value (%) cached by the sampling routine */
extern float humidity;

/**
 * @brief Assemble a `SensorPacket` using the most recent sensor readings.
 *
 * This function reads sensors as required (via DHT handler or cached
 * values) and returns a populated `SensorPacket` ready to be enqueued or
 * sent over BLE. It should be inexpensive and non-blocking where possible.
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
// uint32_t getTimestamp();

#endif // SENSOR_PACKAGE_MANAGER_H
