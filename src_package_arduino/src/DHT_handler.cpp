#include "DHT_handler.h"

const size_t QUEUE_SIZE = 100;
const uint8_t DHT_PIN = 2;
const uint8_t DHT_TYPE = DHT11;

float temperature = 0.0;
float humidity = 0.0;

SensorPacket buffer[QUEUE_SIZE];

size_t queue_head = 0, queue_tail = 0, queue_count = 0;

DHT dht(DHT_PIN, DHT_TYPE);

void initializeDHT()
{
    dht.begin();
    Serial.println("DHT initialized!");
}

void readDHT(SensorPacket &packet)
{
    packet.temperature = dht.readTemperature();
    packet.humidity = dht.readHumidity();
}

bool addPacketToBuffer(const SensorPacket &packet)
{
    if (queue_count < QUEUE_SIZE)
    {
        buffer[queue_tail] = packet;

        queue_tail = (queue_tail + 1) % QUEUE_SIZE;
        queue_count++;

        return true;
    }

    return false;
}

bool getPacketFromBuffer(SensorPacket &packet)
{
    if (queue_count > 0)
    {
        packet = buffer[queue_head];

        queue_head = (queue_head + 1) % QUEUE_SIZE;
        queue_count--;

        return true;
    }

    return false;
}

void flushBuffer()
{
    queue_head = 0;
    queue_tail = 0;
    queue_count = 0;

    for (size_t i = 0; i < QUEUE_SIZE; i++)
    {
        buffer[i] = SensorPacket{};
    }
}