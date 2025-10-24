/**
 * @file sensor_packet_from_sensor.h
 * @brief Defines the data structure for sensor packets received from Arduino sensors.
 *
 * This header declares the `SensorPacket` struct used for communication between
 * sensors and the broker/backend system. The struct is packed to ensure there
 * is no padding between fields, making it suitable for BLE or other binary
 * transport protocols.
 *
 * Example usage:
 * @code
 * SensorPacket packet;
 * packet.sensor_id = 1;
 * packet.temperature = 23.5;
 * packet.humidity = 45.0;
 * packet.sensor_timestamp = millis();
 * @endcode
 */

#ifndef SENSOR_PACKET_FROM_SENSOR_H
#define SENSOR_PACKET_FROM_SENSOR_H

#include <Arduino.h> // For standard types like uint8_t, uint32_t, float

/**
 * @brief Packed structure representing a sensor data packet.
 *
 * Sent from sensors to the broker/backend. Packed to ensure fixed-size
 * layout without padding for reliable transmission over BLE or other protocols.
 */
#pragma pack(push, 1)
typedef struct __attribute__((packed))
{
    uint8_t sensor_id;         /**< Unique sensor identifier */
    uint32_t sensor_timestamp; /**< Timestamp from the sensor (milliseconds) */
    float temperature;         /**< Temperature reading (Celsius) */
    float humidity;            /**< Humidity reading (%) */
} SensorPacket;
#pragma pack(pop)

#endif // SENSOR_PACKET_FROM_SENSOR_H
