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
 * @brief Add a SensorPacket to the circular buffer with automatic recovery and overflow handling.
 *
 * This function adds a new packet to the circular buffer and ensures buffer integrity.
 * It includes safety checks to detect and repair buffer corruption, and gracefully handles
 * buffer overflows by automatically dropping the oldest packet instead of rejecting new data.
 *
 * Behavior:
 * - If the buffer state is invalid (`isBufferValid()` returns false), it is flushed to prevent
 * corruption.
 * - If the buffer is full, the oldest packet is dropped (`dropOldestPacket()`) before inserting the
 * new one.
 * - The function always attempts to insert the new packet afterward.
 *
 * @param packet The SensorPacket to add to the buffer.
 *
 * @return true Always returns true as long as the packet is added to the buffer, even if the buffer
 * was flushed due to corruption or the oldest packet had to be dropped.
 *
 * @note This function never returns false; it will always attempt to recover from
 *       buffer corruption (by flushing) or overflow (by dropping the oldest packet)
 *       before adding the new packet, and will always return true after the attempt.
 * @note If frequent warnings about buffer fullness appear, consider increasing `QUEUE_SIZE`.
 *
 * @code
 * SensorPacket packet;
 * packet.sensor_id = 1;
 * packet.temperature = 22.5;
 * packet.humidity = 45.0;
 *
 * if (addPacketToBuffer(packet)) {
 *     Serial.println("Packet added safely to buffer");
 * }
 * @endcode
 */
bool addPacketToBuffer(const SensorPacket &packet)
{
    if (!isBufferValid())
    {
        Serial.println("ERROR: Buffer corruption detected. Flushing...");
        flushBuffer();
    }

    if (queue_count >= QUEUE_SIZE)
    {
        Serial.println("WARNING: Buffer full — dropping oldest packet.");
        dropOldestPacket();
    }

    buffer[queue_tail] = packet;
    queue_tail = (queue_tail + 1) % QUEUE_SIZE;
    queue_count++;

    return true;
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
 * @brief Drop the oldest packet from the circular buffer.
 *
 * @return true if a packet was successfully dropped.
 * @return false if the buffer is empty.
 *
 * @code
 * if (!dropOldestPacket()) {
 *     Serial.println("Buffer empty");
 * }
 * @endcode
 */
bool dropOldestPacket()
{
    if (queue_count > 0)
    {
        queue_head = (queue_head + 1) % QUEUE_SIZE;
        queue_count--;
        return true;
    }
    return false;
}

/**
 * @brief Validate the integrity of the buffer state.
 *
 * Ensures head, tail, and count are within valid ranges.
 *
 * @return true if the buffer state is valid.
 * @return false if the buffer state is invalid.
 *
 * @code
 * if (!isBufferValid()) {
 *     Serial.println("Buffer state invalid!");
 * }
 * @endcode
 */
bool isBufferValid()
{
    return (queue_count <= QUEUE_SIZE) && (queue_head < QUEUE_SIZE) && (queue_tail < QUEUE_SIZE);
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
