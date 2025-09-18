#include "DHT_handler.h"

// Pins and DHT type
const uint8_t DHT_PIN = 2;
const uint8_t DHT_TYPE = DHT11;

// Sensor variables
uint8_t sensor_id = 0;
float temperature = 0.0;
float humidity = 0.0;

// Circular buffer
SensorPacket buffer[QUEUE_SIZE];
size_t queue_head = 0, queue_tail = 0, queue_count = 0;

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

// Assemble a SensorPacket using current sensor data
SensorPacket assembleSensorPacket() {}

// Add a packet to the circular buffer
bool addPacketToBuffer(const SensorPacket &packet)
{
    if (queue_count < QUEUE_SIZE)
    {
        buffer[queue_tail] = packet;

        queue_tail = (queue_tail + 1) % QUEUE_SIZE;
        queue_count++;

        return true;
    }

    return false; // buffer is full
}

// Retrieve a packet from the circular buffer
bool getPacketFromBuffer(SensorPacket &packet)
{
    if (queue_count > 0)
    {
        packet = buffer[queue_head];

        queue_head = (queue_head + 1) % QUEUE_SIZE;
        queue_count--;

        return true;
    }

    return false; // buffer is empty
}

// Properly cleans the buffer.
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

// currently not implemented
uint32_t getTimestamp() {}