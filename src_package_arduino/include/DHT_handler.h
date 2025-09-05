#ifndef DHT_HANDLER_H
#define DHT_HANDLER_H

#include "DHT.h"
#include <Arduino.h>

extern const uint8_t DHT_PIN;
extern const uint8_t DHT_TYPE;

extern float temperature;
extern float humidity;

typedef struct
{
    // uint32_t local_packet_id; should we have this?
    uint8_t sensor_id;
    uint32_t sensor_timestamp;
    float temperature;
    float humidity;
    uint32_t server_package_id;
    //? truck_id;
} SensorPacket;

extern DHT dht;

void initializeDHT();

#endif