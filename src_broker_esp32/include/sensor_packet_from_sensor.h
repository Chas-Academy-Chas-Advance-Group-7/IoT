#ifndef SENSOR_PACKET_FROM_SENSOR_H
#define SENSOR_PACKET_FROM_SENSOR_H

#include <Arduino.h> // For standard types like uint8_t, uint32_t, float

// Sensor packet structure received from Arduino sensors
typedef struct __attribute__((packed))
{
    uint8_t sensor_id;
    uint32_t sensor_timestamp;
    float temperature;
    float humidity;
    uint32_t server_package_id;
    uint16_t package_sequence_number;
} SensorPacket;

#endif
