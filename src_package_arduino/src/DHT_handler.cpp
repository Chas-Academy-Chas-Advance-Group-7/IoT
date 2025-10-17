/**
 * @file DHT_handler.cpp
 * @brief Handles reading temperature and humidity from a DHT sensor.
 *
 * Provides functions to initialize a DHT sensor and read its temperature
 * and humidity values into a `SensorPacket` structure.
 *
 * Example usage:
 * @code
 * SensorPacket packet;
 * initializeDHT();
 *
 * if (readDHT(packet)) {
 *     Serial.printf("Temperature: %.2f C, Humidity: %.2f %%\n", packet.temperature,
 * packet.humidity); } else { Serial.println("Failed to read DHT sensor");
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
