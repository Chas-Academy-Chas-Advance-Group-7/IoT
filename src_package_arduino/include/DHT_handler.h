#ifndef DHT_HANDLER_H
#define DHT_HANDLER_H

#include "DHT.h"
#include "sensor_package_manager.h"
#include <Arduino.h>

// DHT sensor object
extern DHT dht;

// Pins and type
extern const uint8_t DHT_PIN;
extern const uint8_t DHT_TYPE;

// Function declarations
void initializeDHT();
bool readDHT(SensorPacket &packet);

#endif
