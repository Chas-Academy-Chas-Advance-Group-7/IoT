#ifndef SENSOR_STATE_MANAGER_H
#define SENSOR_STATE_MANAGER_H

#include "DHT_handler.h"
#include "buffer_manager.h"
#include "sensor_package_manager.h"
#include <Arduino.h>

// Defines the states of the sensor
enum class sensor_state
{
    IDLE,
    CREATE_AND_BUFFER_PACKET,
    TRANSFER_PACKET_BATCH,
    UPDATE_SERVER_PACKAGE_ID,
    ERROR_STATE
};

// Declare the current state
extern sensor_state current_sensor_state;

// Function declarations
void determineSensorState();
#endif