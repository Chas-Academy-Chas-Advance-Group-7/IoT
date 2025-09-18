#ifndef DHT_HANDLER_H
#define DHT_HANDLER_H

#include "DHT.h"
#include <Arduino.h>

extern const size_t QUEUE_SIZE;
extern const uint8_t DHT_PIN;
extern const uint8_t DHT_TYPE;

extern float temperature;
extern float humidity;

typedef struct
{
    uint8_t sensor_id;
    uint32_t sensor_timestamp;
    float temperature;
    float humidity;
    uint32_t server_package_id;
    uint16_t package_sequence_number;
} SensorPacket;

extern SensorPacket buffer[QUEUE_SIZE];

extern size_t queue_head, queue_tail, queue_count;

extern DHT dht;

void initializeDHT();
void readDHT(SensorPacket &packet);
bool addPacketToBuffer(SensorPacket &packet);
bool getPacketFromBuffer(SensorPacket &packet);

#endif