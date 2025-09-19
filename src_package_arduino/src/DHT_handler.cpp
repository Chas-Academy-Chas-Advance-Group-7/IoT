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
void readDHT(SensorPacket &packet)
{
    packet.temperature = dht.readTemperature();
    packet.humidity = dht.readHumidity();
}
