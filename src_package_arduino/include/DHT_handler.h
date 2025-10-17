/**
 * @file DHT_handler.h
 * @brief Header for DHT sensor handling on the sensor unit.
 *
 * Declares the DHT sensor object, pins, sensor type, and functions
 * for initializing and reading temperature and humidity into a `SensorPacket`.
 *
 * Example usage:
 * @code
 * #include "DHT_handler.h"
 *
 * initializeDHT();
 * SensorPacket packet;
 * if (readDHT(packet)) {
 *     Serial.printf("Temp: %.2f C, Humidity: %.2f %%\n", packet.temperature, packet.humidity);
 * }
 * @endcode
 */

#ifndef DHT_HANDLER_H
#define DHT_HANDLER_H

#include "DHT.h"
#include "sensor_package_manager.h"
#include <Arduino.h>

/** DHT sensor object used for readings */
extern DHT dht;

/** GPIO pin connected to the DHT sensor */
extern const uint8_t DHT_PIN;

/** DHT sensor type (e.g., DHT11 or DHT22) */
extern const uint8_t DHT_TYPE;

/**
 * @brief Initialize the DHT sensor for readings.
 *
 * Must be called before attempting to read temperature or humidity.
 */
void initializeDHT();

/**
 * @brief Read temperature and humidity from the DHT sensor.
 *
 * Stores the readings into a provided `SensorPacket`.
 *
 * @param packet Reference to a SensorPacket to store readings.
 * @return true if the reading succeeded.
 * @return false if the reading failed (e.g., sensor error or invalid values).
 */
bool readDHT(SensorPacket &packet);

#endif // DHT_HANDLER_H
