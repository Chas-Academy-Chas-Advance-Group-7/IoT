#include "buffer_manager.h"

// Circular buffer
SensorPacket buffer[QUEUE_SIZE];
size_t queue_head = 0, queue_tail = 0, queue_count = 0;

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