/**
 * @file buffer_manager.cpp
 * @brief Circular buffer management for storing SensorPacket data.
 *
 * Provides functions to add, retrieve, peek, commit, and flush sensor packets
 * in a fixed-size circular buffer. This is useful for temporary storage of
 * sensor data before sending over BLE or network.
 *
 * Example usage:
 * @code
 * SensorPacket packet;
 * if (addPacketToBuffer(packet)) {
 *     Serial.println("Packet added");
 * }
 *
 * if (peekPacketFromBuffer(packet)) {
 *     // Inspect packet without removing
 *     commitPacketRemoval(); // Remove after processing
 * }
 * flushBuffer(); // Clear buffer
 * @endcode
 */

#include "buffer_manager.h"

/** Circular buffer to store sensor packets */
SensorPacket buffer[QUEUE_SIZE];

/** Index of the head of the buffer */
size_t queue_head = 0;

/** Index of the tail of the buffer */
size_t queue_tail = 0;

/** Current number of elements in the buffer */
size_t queue_count = 0;

/**
 * @brief Add a packet to the circular buffer.
 *
 * @param packet SensorPacket to add.
 * @return true if the packet was added successfully.
 * @return false if the buffer is full.
 *
 * @code
 * SensorPacket packet = {...};
 * if (!addPacketToBuffer(packet)) {
 *     Serial.println("Buffer full");
 * }
 * @endcode
 */
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

/**
 * @brief Retrieve and remove the oldest packet from the circular buffer.
 *
 * @param packet Reference to store the retrieved packet.
 * @return true if a packet was successfully retrieved.
 * @return false if the buffer is empty.
 *
 * @code
 * SensorPacket packet;
 * if (getPacketFromBuffer(packet)) {
 *     Serial.println(packet.sensor_id);
 * }
 * @endcode
 */
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

/**
 * @brief Peek at the oldest packet without removing it from the buffer.
 *
 * @param packet Reference to store the peeked packet.
 * @return true if a packet is available to peek.
 * @return false if the buffer is empty.
 *
 * @code
 * SensorPacket packet;
 * if (peekPacketFromBuffer(packet)) {
 *     Serial.println(packet.sensor_id); // inspect without removal
 * }
 * @endcode
 */
bool peekPacketFromBuffer(SensorPacket &packet)
{
    if (queue_count > 0)
    {
        packet = buffer[queue_head];
        return true;
    }
    return false; // Buffer is empty
}

/**
 * @brief Commit the removal of the last peeked packet.
 *
 * Advances the head pointer and decreases the buffer count.
 *
 * @code
 * if (peekPacketFromBuffer(packet)) {
 *     process(packet);
 *     commitPacketRemoval();
 * }
 * @endcode
 */
void commitPacketRemoval()
{
    if (queue_count > 0)
    {
        queue_head = (queue_head + 1) % QUEUE_SIZE;
        queue_count--;
    }
}

/**
 * @brief Clear the entire circular buffer.
 *
 * Resets head, tail, and count, and clears all stored packets.
 *
 * @code
 * flushBuffer(); // Empty the buffer
 * @endcode
 */
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
