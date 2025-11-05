/**
 * @file DHT_handler.cpp
 * @brief Handles reading temperature and humidity from a DHT sensor.
 *
 * Implementation notes:
 * - The DHT sensor is relatively slow and must not be polled aggressively.
 *   Respect the sensor's minimum sampling interval (typically ~2 seconds
 *   depending on model) to avoid invalid reads.
 * - `readDHT()` may block while waiting for the sensor response; keep
 *   calls infrequent and avoid calling from time-critical contexts.
 * - The function returns a boolean success indicator; failed reads do not
 *   attempt complex recovery and are left to higher-level logic to handle
 *   (retry, drop, or escalate to ERROR_STATE).
 *
 * Example usage:
 * @code
 * SensorPacket packet;
 * initializeDHT();
 *
 * if (readDHT(packet)) {
 *     Serial.printf("Temperature: %.2f C, Humidity: %.2f %%\n",
 *                   packet.temperature, packet.humidity);
 * } else {
 *     Serial.println("Failed to read DHT sensor");
 * }
 * @endcode
 */

#include "DHT_handler.h"
#include "sensor_package_manager.h"

/** GPIO pin where the DHT sensor is connected */
const uint8_t DHT_PIN = 2;

/** Type of DHT sensor (e.g., DHT11, DHT22) */
const uint8_t DHT_TYPE = DHT11;

/** DHT object used for reading sensor data */
DHT dht(DHT_PIN, DHT_TYPE);

/**
 * @brief Initialize the DHT sensor.
 *
 * Sets up the sensor for reading temperature and humidity.
 *
 * @code
 * initializeDHT();
 * @endcode
 */
void initializeDHT()
{
    dht.begin();
    Serial.println("DHT initialized!");
}

/**
 * @brief Read temperature and humidity from the DHT sensor and store in a packet.
 *
 * @param packet Reference to a SensorPacket to store the readings.
 * @return true if the reading was successful.
 * @return false if the reading failed (e.g., sensor error or invalid values).
 *
 * @code
 * SensorPacket packet;
 * if (readDHT(packet)) {
 *     Serial.printf("Temp: %.2f C, Humidity: %.2f %%\n", packet.temperature, packet.humidity);
 * }
 * @endcode
 */
bool readDHT(SensorPacket &packet)
{
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
        return false; // reading failed
    }

    packet.temperature = temperature;
    packet.humidity = humidity;
    return true; // Successful reading
}
