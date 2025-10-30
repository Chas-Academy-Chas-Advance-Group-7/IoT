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
 * Responsibilities:
 * - Call `dht.begin()` and perform any board-specific setup.
 * - Should be invoked once during `setup()` before calling `readDHT()`.
 *
 * Note: DHT sensors require a minimum interval between readings (commonly
 * ~2 seconds for DHT11/DHT22). Do not call `readDHT()` more frequently than
 * the sensor supports — the function does not internally throttle calls.
 */
void initializeDHT();

/**
 * @brief Read temperature and humidity from the DHT sensor and store in a packet.
 *
 * Behaviour and contract:
 * - Attempts to read temperature and humidity via the `DHT` object.
 * - If the read is successful (values are not NaN) the function fills
 *   `packet.temperature` and `packet.humidity` and returns true.
 * - On failure the function returns false and leaves the provided `packet`
 *   in an indeterminate or previously-set state; callers should handle
 *   failures explicitly (e.g., retry later or abort packet creation).
 *
 * Timing/Blocking:
 * - DHT reads can block for the sensor's response time. Keep calls
 *   infrequent and avoid using in time-critical or high-frequency loops.
 *
 * @param packet Reference to a SensorPacket to store readings.
 * @return true if the reading succeeded.
 * @return false if the reading failed (e.g., sensor error or invalid values).
 */
bool readDHT(SensorPacket &packet);

#endif // DHT_HANDLER_H
