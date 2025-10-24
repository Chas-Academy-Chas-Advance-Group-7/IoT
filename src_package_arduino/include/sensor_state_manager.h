/**
 * @file sensor_state_manager.h
 * @brief Manages the state machine of the sensor unit.
 *
 * Declares sensor states, the current state, and functions
 * for handling each state in the sensor firmware.
 *
 * Example usage:
 * @code
 * determineSensorState();
 * state_CreateAndBufferPacket();
 * state_TransferPacketBatch();
 * @endcode
 */

#ifndef SENSOR_STATE_MANAGER_H
#define SENSOR_STATE_MANAGER_H

#include "DHT_handler.h"
#include "bluetooth_manager.h"
#include "buffer_manager.h"
#include "sensor_package_manager.h"
#include <Arduino.h>

/** Maximum number of consecutive failed BLE transmission attempts */
constexpr int MAX_FAILED_ATTEMPTS = 3;

/**
 * @brief Enumerates possible states of the sensor.
 */
enum class sensor_state
{
    IDLE,                     /**< Waiting or idle state */
    CREATE_AND_BUFFER_PACKET, /**< Creating a sensor packet and buffering it */
    TRANSFER_PACKET_BATCH,    /**< Sending buffered packets via BLE */
    BLE_MANAGEMENT,           /**< Manage BLE connections (placeholder) */
    READ_FLASH_MEMORY,        /**< Read state or data from flash memory (placeholder) */
    // WRITE_FLASH_MEMORY_BUFFER_BATCH, /**< Write buffer batch to flash (placeholder) */
    ERROR_STATE /**< Error state due to transmission or buffer failure */
};

/** Current state of the sensor unit */
extern sensor_state current_sensor_state;

extern sensor_state previous_state_to_error_state;

/**
 * @brief Determine and handle transitions between sensor states.
 *
 * Call periodically in the main loop or task.
 */
void determineSensorState();

/**
 * @brief Transition to the ERROR_STATE.
 *
 * Call this function to handle error transitions.
 * Updates `current_sensor_state` to ERROR_STATE.
 */
void transitionToErrorState();

/**
 * @brief Create a new SensorPacket and add it to the circular buffer.
 *
 * Updates `current_sensor_state` depending on success or buffer failure.
 */
void state_CreateAndBufferPacket();

/**
 * @brief Send one packet from the buffer via BLE.
 *
 * Uses peek/commit strategy, respects per-packet intervals, tracks failures,
 * and updates `current_sensor_state`.
 */
void state_TransferPacketBatch();

void state_BLEManagement();

/**
 * @brief Handle the ERROR_STATE.
 *
 * Performs error diagnostics and can include recovery logic.
 */
void state_ErrorState();

/**
 * @brief Read sensor state from flash memory.
 *
 * Placeholder for future implementation.
 */
void state_ReadFlashMemory();

// Placeholder for batch write to flash memory
// void state_WriteFlashMemoryBufferBatch();

#endif // SENSOR_STATE_MANAGER_H
