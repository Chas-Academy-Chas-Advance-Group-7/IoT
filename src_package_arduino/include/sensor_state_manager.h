/**
 * @file sensor_state_manager.h
 * @brief Sensor unit state machine API and documentation.
 *
 * This header declares the sensor state enumeration, the global
 * state variables used by the firmware, and the public state-handler
 * functions that the main loop (or tasked runner) should call.
 *
 * Design contract (concise):
 * - Inputs: hardware readings provided by `DHT_handler`, BLE stack status
 *   exposed by `bluetooth_manager`, and the packet circular buffer API
 *   in `buffer_manager` / `sensor_package_manager`.
 * - Outputs / side-effects: fills the circular buffer with `SensorPacket`s,
 *   performs BLE notifications, updates persistent indicators (flash) and
 *   moves `current_sensor_state` as transitions occur.
 * - Error modes: network/BLE failures, buffer-full, sensor read failures.
 * - Success criteria: packets are created and enqueued; buffered packets are
 *   delivered over BLE to a central within retry limits.
 *
 * State machine behaviour summary and expectations:
 * - `CREATE_AND_BUFFER_PACKET` attempts to sample sensors and enqueue a
 * *  packet. On success it typically transitions to `TRANSFER_PACKET_BATCH`
 * *  (or `IDLE` depending on timing policy). On enqueue failure it may
 * *  remain or transition to `ERROR_STATE` after retry.
 * - `TRANSFER_PACKET_BATCH` peeks at the head of the buffer and performs a
 * *  BLE notification. The buffer uses a peek/commit pattern: successful
 * *  delivery commits (removes) the packet, failures increment a retry
 * *  counter. Repeated failures beyond `MAX_FAILED_ATTEMPTS` should lead to
 * *  `ERROR_STATE`.
 * - `BLE_MANAGEMENT` owns BLE lifecycle: advertising, accepting connections,
 * *  and reconnection attempts. It should be resilient and idempotent if
 * *  called repeatedly.
 * - `READ_FLASH_MEMORY` is reserved for restoring state on boot or after
 * *  error recovery and is a no-op until implemented.
 * - `ERROR_STATE` collects diagnostics, may attempt recovery back to the
 * *  previous stable state, or require manual intervention.
 *
 * Example usage (in `loop()`):
 * @code
 * determineSensorState();          // recompute next state based on timers/flags
 * switch (current_sensor_state) {
 *
 *   case sensor_state::CREATE_AND_BUFFER_PACKET:
 *   state_CreateAndBufferPacket();
 *   break;
 *
 *   case sensor_state::TRANSFER_PACKET_BATCH:
 *   state_TransferPacketBatch();
 *   break;
 *
 *   case sensor_state::BLE_MANAGEMENT:
 *   state_BLEManagement();
 *   break;
 *
 *   case sensor_state::READ_FLASH_MEMORY:
 *   state_ReadFlashMemory(); // currently not implemented
 *   break;
 *
 *   case sensor_state::ERROR_STATE:
 *   state_ErrorState();
 *   break;
 *
 *   default: IDLE / no-op
 *   break;
 * }
 * @endcode
 */

#ifndef SENSOR_STATE_MANAGER_H
#define SENSOR_STATE_MANAGER_H

#include "DHT_handler.h"
#include "bluetooth_manager.h"
#include "buffer_manager.h"
#include "sensor_package_manager.h"
#include <Arduino.h>

/** Maximum number of consecutive failed BLE transmission attempts
 *
 * If the transfer routine records more than this many consecutive failures
 * for the same packet it is considered unrecoverable and the machine
 * should transition into `ERROR_STATE` for diagnostics/recovery.
 */
constexpr int MAX_FAILED_ATTEMPTS = 3;

/**
 * @brief Enumeration of possible sensor states.
 *
 * Each state represents a specific operation or condition
 * within the sensor firmware.
 *
 * Example usage:
 * @code
 * sensor_state current_sensor_state = sensor_state::IDLE;
 *
 * current_sensor_state = sensor_state::CREATE_AND_BUFFER_PACKET;
 *
 * current_sensor_state = sensor_state::TRANSFER_PACKET_BATCH;
 *
 * current_sensor_state = sensor_state::BLE_MANAGEMENT;
 *
 * current_sensor_state = sensor_state::READ_FLASH_MEMORY;
 *
 * current_sensor_state = sensor_state::ERROR_STATE;
 * @endcode
 *
 *
 */
enum class sensor_state
{
    IDLE,
    /**<
     * No immediate action required. The machine stays here while waiting for
     * timers, sampling intervals or external triggers. Implementation detail:
     * calling any state handler while in IDLE should be a no-op.
     */

    CREATE_AND_BUFFER_PACKET,
    /**<
     * Read sensors (via `DHT_handler`) and build a `SensorPacket` using
     * `sensor_package_manager`. On success enqueue the packet with
     * `buffer_manager::addPacketToBuffer`. Side effects: increments buffer
     * count. On enqueue failure, the implementation may retry or set
     * `current_sensor_state` to `ERROR_STATE` after configurable attempts.
     */

    TRANSFER_PACKET_BATCH,
    /**<
     * Attempt to transmit buffered packets over BLE using a peek/commit
     * strategy. On a successful notification the packet is removed from the
     * buffer. On transient failure the packet remains and a retry counter is
     * incremented. If the retry counter exceeds `MAX_FAILED_ATTEMPTS`, the
     * machine should move to `ERROR_STATE`.
     */

    BLE_MANAGEMENT,
    /**<
     * Responsible for BLE stack lifecycle: advertising, connection handling
     * and re-advertising on disconnect. This state should be safe to call
     * repeatedly and must not block for long periods.
     */

    READ_FLASH_MEMORY,
    /**<
     * Reserved: restore persistent configuration or queued packets from
     * flash after a reset / power-cycle. Currently a placeholder.
     */

    // WRITE_FLASH_MEMORY_BUFFER_BATCH, /**< Write buffer batch to flash (placeholder) */

    ERROR_STATE
    /**<
     * Terminal/intermediate error handling state. Implementations should
     * gather diagnostic info (error counters, last-failed-packet) and may
     * attempt automated recovery to `previous_state_to_error_state` or to
     * a safe `IDLE` state after manual/automatic clearing.
     */
};

/** Current state of the sensor unit */
extern sensor_state current_sensor_state;

/** The state that was active immediately prior to entering ERROR_STATE.
 *
 * Useful for recovery: the error handler can examine this and attempt to
 * continue from the previous state once diagnostics/recovery are complete.
 */
extern sensor_state previous_state_to_error_state;

/**
 * @brief Re-evaluate timers, flags and inputs to select the next state.
 *
 * Responsibilities:
 * - Inspect sampling timers, buffer occupancy and BLE availability.
 * - Set `current_sensor_state` to the next state. Do not perform long
 *   blocking work in this function; it should only decide the next step.
 *
 * When to call: from `loop()` or the scheduler at a regular cadence (e.g.
 * every 100ms). This function does not itself call the state's handler.
 */
void determineSensorState();

/**
 * @brief Enter the `ERROR_STATE`.
 *
 * This function records `previous_state_to_error_state` and updates
 * `current_sensor_state`. Prefer calling this helper rather than setting
 * the `current_sensor_state` directly so diagnostic context is preserved.
 *
 * Side effects: may set internal error counters or timestamps used by the
 * error handler.
 */
void transitionToErrorState();

/**
 * @brief Try sampling sensors and enqueue a new `SensorPacket`.
 *
 * Behavior / contract:
 * - Calls into `DHT_handler` to acquire temperature/humidity.
 * - Builds a `SensorPacket` via `sensor_package_manager`.
 * - Calls `buffer_manager::addPacketToBuffer` and handles buffer-full
 *   conditions.
 * - On success: the function should advance the machine (via
 *   `determineSensorState()` or by updating `current_sensor_state`) to
 *   `TRANSFER_PACKET_BATCH` or `IDLE` depending on the configured cadence.
 * - On recoverable failure (transient sensor read): it may retry a small
 *   fixed number of times before returning control to the main loop.
 * - On unrecoverable failure (buffer full after retries) it should call
 *   `transitionToErrorState()` to surface the problem.
 */
void state_CreateAndBufferPacket();

/**
 * @brief Transfer the next buffered packet via BLE notification.
 *
 * Contract and behavior:
 * - Uses buffer peek to obtain the next packet without removing it.
 * - Attempts to notify the connected BLE central with the packet payload.
 * - On success: commits (removes) the packet from the buffer and resets the
 *   packet-specific retry counter.
 * - On transient failure: increments a per-packet or global retry counter.
 * - If retries for the same packet exceed `MAX_FAILED_ATTEMPTS`, calls
 *   `transitionToErrorState()` to allow diagnostics/recovery.
 *
 * Timing: this function should enforce any inter-packet spacing required
 * by the BLE stack (do not block long periods; use timestamps to enforce
 * spacing when called frequently).
 */
void state_TransferPacketBatch();

/**
 * @brief Ensure BLE is in the expected lifecycle state (advertising/connected).
 *
 * Responsibilities:
 * - Initialize BLE stack early in boot.
 * - Start/stop advertising as required.
 * - Respond to connect/disconnect events and attempt limited reconnects.
 *
 * Non-goals: this function should not implement high-level packet logic.
 * Keep it focused on stack/lifecycle concerns and fast-returning checks.
 */
void state_BLEManagement();

/**
 * @brief Run diagnostics and recovery paths for unrecoverable errors.
 *
 * Typical actions:
 * - Log error counters and the last failed packet (if available).
 * - Attempt safe recovery steps (reset BLE, flush or persist buffer to
 *   flash, attempt to clear transient errors).
 * - If recovery succeeds, set `current_sensor_state` back to the saved
 *   `previous_state_to_error_state` or `IDLE`.
 * - If recovery fails repeatedly, remain in `ERROR_STATE` and surface
 *   the failure (e.g. via LED or retained log) until manual intervention.
 */
void state_ErrorState();

/**
 * @brief Restore state from flash storage (placeholder).
 *
 * Intended to restore queued packets or configuration after reboot. This
 * is currently a stub and should be implemented with careful attention to
 * atomicity and power-loss safety.
 */
void state_ReadFlashMemory();

// Placeholder for batch write to flash memory
// void state_WriteFlashMemoryBufferBatch();

#endif // SENSOR_STATE_MANAGER_H
