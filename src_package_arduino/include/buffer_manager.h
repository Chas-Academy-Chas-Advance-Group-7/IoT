#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "sensor_package_manager.h"
#include <Arduino.h>

// Queue size constant
constexpr size_t QUEUE_SIZE = 100;

// Circular buffer for sensor packets
extern SensorPacket buffer[QUEUE_SIZE];
extern size_t queue_head;
extern size_t queue_tail;
extern size_t queue_count;

// Function declarations
bool addPacketToBuffer(const SensorPacket &packet);
bool getPacketFromBuffer(SensorPacket &packet);
bool peekPacketFromBuffer(SensorPacket &packet);
void commitPacketRemoval();
void flushBuffer();

#endif