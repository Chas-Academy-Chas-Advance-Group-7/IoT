/**
 * @file buffer_manager.h
 * @brief Circular buffer interface for storing SensorPacket data.
 *
 * This module implements a fixed-size circular buffer used by the sensor
 * firmware to queue outgoing `SensorPacket`s prior to transmission. The
 * API intentionally exposes simple operations (add/peek/commit/get/drop)
 * so that higher-level logic can implement retry, persistence, and
 * transmission flows.
 *
 * Contract / invariants:
 * - The buffer is sized by `QUEUE_SIZE` and stores up to that many
 *   `SensorPacket` elements.
 * - `queue_count` always holds the number of valid elements in the buffer
 *   (0 <= queue_count <= QUEUE_SIZE).
 * - `queue_head` points to the index of the oldest valid packet (if any).
 * - `queue_tail` points to the next free slot where a packet will be
 *   written when adding.
 * - After a successful `peekPacketFromBuffer()` the caller must call
 *   `commitPacketRemoval()` to remove that packet or call `dropOldestPacket()`
 *   if the packet should be discarded. Peeking without committing leaves the
 *   packet in the buffer for subsequent attempts.
 *
 * Threading / safety notes:
 * - The sensor firmware runs on a single-threaded Arduino environment by
 *   default. These APIs are NOT safe for concurrent access from multiple
 *   threads or ISRs. If you plan to call these functions from interrupt
 *   contexts or multiple tasks, protect access with a mutex or disable
 *   interrupts around buffer operations.
 *
 * Error behaviours:
 * - `addPacketToBuffer()` returns false when the buffer is full; callers
 *   should handle this as a recoverable or fatal condition depending on
 *   the application (e.g. retry, drop oldest, or escalate to ERROR_STATE).
 * - `getPacketFromBuffer()` / `peekPacketFromBuffer()` return false when
 *   the buffer is empty.
 *
 * Example usage (peek/commit pattern):
 * @code
 * SensorPacket pkt;
 * if (peekPacketFromBuffer(pkt)) {
 *   // try to transmit pkt over BLE
 *   if (sendOverBLE(pkt)) {
 *     // only remove when transmission confirmed
 *     commitPacketRemoval();
 *   } else {
 *     // leave the packet in the buffer for retry
 *   }
 * }
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
