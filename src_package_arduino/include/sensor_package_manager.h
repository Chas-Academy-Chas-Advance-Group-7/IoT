#ifndef SENSOR_PACKAGE_MANAGER_H
#define SENSOR_PACKAGE_MANAGER_H

#include <Arduino.h>

// Sensor packet structure
typedef struct __attribute__((packed))
{
    uint8_t sensor_id;
    uint32_t sensor_timestamp;
    float temperature;
    float humidity;
    uint32_t server_package_id;
    uint16_t package_sequence_number;
} SensorPacket;

// Sensor variables
extern uint8_t sensor_id;
extern float temperature;
extern float humidity;
extern uint32_t server_package_id;
extern uint16_t package_sequence_number;

// Function declarations
SensorPacket assembleSensorPacket();
uint32_t getTimestamp();

#endif