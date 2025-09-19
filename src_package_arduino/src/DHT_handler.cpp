#include "DHT_handler.h"
#include "sensor_package_manager.h"

// Pins and DHT type
const uint8_t DHT_PIN = 2;
const uint8_t DHT_TYPE = DHT11;

// DHT object
DHT dht(DHT_PIN, DHT_TYPE);

// Initialize the DHT sensor
void initializeDHT()
{
    dht.begin();
    Serial.println("DHT initialized!");
}

// reads values and store them in a packet
bool readDHT(SensorPacket &packet)
{
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
        return false; // reading failed
    }

    packet.temperature = temperature;
    packet.humidity = humidity;
    return true; // Successful reading
}
