/**
 * @file buffer_manager.h
 * @brief Circular buffer interface for storing SensorPacket data.
 *
 * Declares the fixed-size circular buffer and functions to:
 * - Add packets
 * - Retrieve packets
 * - Peek at packets
 * - Commit packet removal
 * - Flush the buffer
 *
 * Example usage:
 * @code
 * SensorPacket packet;
 * if (addPacketToBuffer(packet)) {
 *     Serial.println("Packet added");
 * }
 *
 * if (peekPacketFromBuffer(packet)) {
 *     // Inspect packet
 *     commitPacketRemoval(); // Remove after processing
 * }
 *
 * flushBuffer(); // Clear the buffer
 * @endcode
 */

#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "sensor_package_manager.h"
#include <Arduino.h>

/** Maximum number of sensor packets in the circular buffer */
constexpr size_t QUEUE_SIZE = 100;

/** Circular buffer storing sensor packets */
extern SensorPacket buffer[QUEUE_SIZE];

/** Index of the head of the buffer */
extern size_t queue_head;

/** Index of the tail of the buffer */
extern size_t queue_tail;

/** Current number of elements in the buffer */
extern size_t queue_count;

/**
 * @brief Add a packet to the circular buffer.
 *
 * @param packet SensorPacket to add.
 * @return true if the packet was added successfully.
 * @return false if the buffer is full.
 */
bool addPacketToBuffer(const SensorPacket &packet);

/**
 * @brief Retrieve and remove the oldest packet from the circular buffer.
 *
 * @param packet Reference to store the retrieved packet.
 * @return true if a packet was successfully retrieved.
 * @return false if the buffer is empty.
 */
bool getPacketFromBuffer(SensorPacket &packet);

/**
 * @brief Peek at the oldest packet without removing it.
 *
 * @param packet Reference to store the peeked packet.
 * @return true if a packet is available to peek.
 * @return false if the buffer is empty.
 */
bool peekPacketFromBuffer(SensorPacket &packet);

/**
 * @brief Commit the removal of the last peeked packet.
 *
 * Advances the head pointer and decreases the buffer count.
 */
void commitPacketRemoval();

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
bool dropOldestPacket();

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
bool isBufferValid();

/**
 * @brief Clear the entire circular buffer.
 *
 * Resets head, tail, and count, and clears all stored packets.
 */
void flushBuffer();

#endif // BUFFER_MANAGER_H
