#include "DHT_handler.h"

const uint8_t DHT_PIN = 2;
const uint8_t DHT_TYPE = DHT11;

float temperature = 0.0;
float humidity = 0.0;

DHT dht(DHT_PIN, DHT_TYPE);

void initializeDHT()
{
    dht.begin();
    Serial.println("DHT initialized!");
}

SensorPacket readDHT(uint8_t sensorId, uint32_t timestamp, uint32_t serverId)
{
    SensorPacket packet;

    packet.sensor_id = sensorId;
    packet.sensor_timestamp = timestamp;
    packet.temperature = dht.readTemperature();
    packet.humidity = dht.readHumidity();
    packet.server_package_id = serverId;

    return packet;
}

//