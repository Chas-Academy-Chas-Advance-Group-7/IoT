#ifndef DHT_HANDLER_H
#define DHT_HANDLER_H

#include "DHT.h"
#include <Arduino.h>

// Queue size constant
constexpr size_t QUEUE_SIZE = 100;

// Sensor packet structure
typedef struct
{
    uint8_t sensor_id;
    uint32_t sensor_timestamp;
    float temperature;
    float humidity;
    uint32_t server_package_id;
    uint16_t package_sequence_number;
} SensorPacket;

// DHT sensor object
extern DHT dht;

// Pins and type
extern const uint8_t DHT_PIN;
extern const uint8_t DHT_TYPE;

// Sensor variables
extern uint8_t sensor_id;
extern float temperature;
extern float humidity;

// Circular buffer for sensor packets
extern SensorPacket buffer[QUEUE_SIZE];
extern size_t queue_head;
extern size_t queue_tail;
extern size_t queue_count;

// Function declarations
void initializeDHT();
void readDHT(SensorPacket &packet);
SensorPacket assembleSensorPacket();
bool addPacketToBuffer(const SensorPacket &packet);
bool getPacketFromBuffer(SensorPacket &packet);
void flushBuffer();
uint32_t getTimestamp();

#endif
