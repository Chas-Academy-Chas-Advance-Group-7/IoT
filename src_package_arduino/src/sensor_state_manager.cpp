/**
 * @file sensor_state_manager.cpp
 * @brief Manages the state machine of the sensor unit for packet creation, buffering, and BLE
 * transfer.
 *
 * Implements state-based behavior for the sensor unit, including:
 * - Creating and buffering sensor packets
 * - Transferring packets over BLE
 * - Handling errors and state transitions
 *
 * Example usage:
 * @code
 * // Create and buffer a packet
 * state_CreateAndBufferPacket();
 *
 * // Periodically call to transfer packets over BLE
 * state_TransferPacketBatch();
 * @endcode
 */

#include "sensor_state_manager.h"

/** Current state of the sensor */
sensor_state current_sensor_state = sensor_state::IDLE;

/** Previous state before entering ERROR_STATE */
sensor_state previous_state_to_error_state = sensor_state::IDLE;

/** BLE initialization flag */
static bool bleInitialized = false;

/** Last time a packet transfer was attempted */
static unsigned long lastTransferCheck = 0;

/** Period between transfer attempts */
const unsigned long TRANSFER_PERIOD = 30000; // e.g. every 30s

/** Last time packet creation was attempted */
static unsigned long lastPacketTime = 0;

/** Period between packet creation attempts */
const unsigned long PACKET_INTERVAL = 10000; // e.g., create a packet every 10 seconds

/**
 * @brief Re-evaluate timers, flags and inputs to select the next state.
 *
 * Responsibilities:
 * - Inspect sampling timers, buffer occupancy and BLE availability.
 * - Set `current_sensor_state` to the next state. This function must be
 *   fast and non-blocking: it only decides the next step and does not
 *   perform long-running operations.
 *
 * When to call: from `loop()` or the scheduler at a regular cadence
 * (for example every 50-200 ms). The real state handler (e.g.
 * `state_CreateAndBufferPacket`) performs the actual work.
 *
 * Side-effects: updates `current_sensor_state` and internal timing
 * variables such as `lastPacketTime` and `lastTransferCheck`.
 */
void determineSensorState()
{
    // Do nothing if in error state — error handler manages recovery
    if (current_sensor_state == sensor_state::ERROR_STATE)
    {
        return;
    }

    // --- 1. Sensor packet creation timing ---
    if (millis() - lastPacketTime >= PACKET_INTERVAL)
    {
        lastPacketTime = millis();
        current_sensor_state = sensor_state::CREATE_AND_BUFFER_PACKET;
        return;
    }

    // --- 2. BLE management priority ---
    // Check if BLE is initialized or central is disconnected
    if (!bleInitialized || !isCentralConnected())
    {
        current_sensor_state = sensor_state::BLE_MANAGEMENT;
        return;
    }

    // --- 3. Packet transfer priority ---
    // Only try sending if there are packets in the buffer
    if (millis() - lastTransferCheck >= TRANSFER_PERIOD && queue_count > 0)
    {
        lastTransferCheck = millis();
        current_sensor_state = sensor_state::TRANSFER_PACKET_BATCH;
        return;
    }

    // --- 4. Flash memory read condition ---
    // Example placeholder — implement your own condition if needed
    /*
    if (shouldReadFlashMemory())
    {
        current_sensor_state = sensor_state::READ_FLASH_MEMORY;
        return;
    }
    */

    // --- 5. Otherwise remain idle ---
    current_sensor_state = sensor_state::IDLE;
}
/**
 * @brief Enter the `ERROR_STATE` and capture context for diagnostics.
 *
 * This helper records the state the system was in prior to the error
 * (in `previous_state_to_error_state`) and sets `current_sensor_state`
 * to `ERROR_STATE`. Use this instead of directly assigning the enum so
 * that diagnostic context is preserved for the error handler.
 *
 * Side-effects: may also be used to increment persistent error counters
 * or capture timestamps (extend as needed in implementations).
 */
void transitionToErrorState()
{
    previous_state_to_error_state = current_sensor_state;
    current_sensor_state = sensor_state::ERROR_STATE;
}

/**
 * @brief Sample sensors, build a `SensorPacket`, and enqueue it.
 *
 * Contract and behavior:
 * - Calls `assembleSensorPacket()` which uses `DHT_handler` to sample
 *   temperature/humidity and populates a `SensorPacket`.
 * - Calls `addPacketToBuffer()` to enqueue the packet using the
 *   circular-buffer API. The buffer uses a peek/commit workflow so
 *   callers should expect `queue_count` to reflect current occupancy.
 * - On success: logs the sample and typically transitions the state
 *   machine to `IDLE` or lets `determineSensorState()` pick the next state.
 * - On buffer failure: logs the error and calls `transitionToErrorState()`.
 *
 * Timing: this function should be called only when sampling is due.
 */
void state_CreateAndBufferPacket()
{
    SensorPacket newPacket = assembleSensorPacket();

    if (!addPacketToBuffer(newPacket))
    {
        Serial.println("Error while adding packet!");
        transitionToErrorState();
    }
    else
    {
        Serial.println("Packet added to buffer.");
        Serial.print("Sensor ID: ");
        Serial.print(newPacket.sensor_id);
        Serial.print(", Temp: ");
        Serial.print(newPacket.temperature);
        Serial.print(", Hum: ");
        Serial.println(newPacket.humidity);

        Serial.print("Buffer count: ");
        Serial.println(queue_count);
        current_sensor_state = sensor_state::IDLE;
    }
}

/**
 * @brief Attempt to transmit the next buffered packet over BLE.
 *
 * Implementation notes and contract:
 * - Uses a peek/commit pattern: `peekPacketFromBuffer()` reads the packet
 *   into a local `SensorPacket` instance. Only after a successful BLE
 *   write is `commitPacketRemoval()` called to remove it from the buffer.
 * - Respects inter-packet spacing via `TRANSFER_INTERVAL` to avoid
 *   overwhelming the BLE stack. Do not block inside this function.
 * - Tracks transient failures with `failed_transmission_attempts_counter`.
 *   When consecutive failures for the same packet exceed
 *   `MAX_FAILED_ATTEMPTS`, `transitionToErrorState()` is called.
 * - If the central is not subscribed or not connected, this function
 *   logs and returns, allowing `BLE_MANAGEMENT` to restore connectivity.
 *
 * Side-effects: may modify `queue_count`, `lastTransferTime` and the
 * local static failure counters.
 */
void state_TransferPacketBatch()
{
    static int failed_transmission_attempts_counter = 0;
    static unsigned long lastTransferTime = 0;
    const unsigned long TRANSFER_INTERVAL = 150; // ms between packets

    // Reset counters if recovering from an error
    static bool recovering_from_error = false;

    if (recovering_from_error)
    {
        failed_transmission_attempts_counter = 0;
        lastTransferTime = millis();
        recovering_from_error = false;
    }

    // Only attempt send if BLE central is connected
    if (isCentralConnected() && queue_count > 0 && millis() - lastTransferTime >= TRANSFER_INTERVAL)
    {
        SensorPacket packet;

        if (peekPacketFromBuffer(packet))
        {
            BLECharacteristic &charRef = getSensorCharacteristic();
            uint8_t buffer[sizeof(SensorPacket)];
            memcpy(buffer, &packet, sizeof(SensorPacket));

            if (charRef.subscribed())
            {
                bool success = charRef.writeValue(buffer, sizeof(SensorPacket));

                if (success)
                {
                    // Commit removal and reset fail counter
                    commitPacketRemoval();
                    failed_transmission_attempts_counter = 0;

                    Serial.print("Sent packet to central: ");
                    Serial.print("Sensor ID: ");
                    Serial.print(packet.sensor_id);
                    Serial.print(", Temp: ");
                    Serial.print(packet.temperature);
                    Serial.print(", Hum: ");
                    Serial.println(packet.humidity);
                }
                else
                {
                    failed_transmission_attempts_counter++;

                    Serial.print("Failed BLE write (attempt ");
                    Serial.print(failed_transmission_attempts_counter);
                    Serial.println(")");

                    // Escalate to ERROR_STATE if repeated failures exceed threshold
                    if (failed_transmission_attempts_counter >= MAX_FAILED_ATTEMPTS)
                    {
                        Serial.println("ERROR: BLE transmission failed too many times!");
                        recovering_from_error = true;
                        transitionToErrorState();
                        return; // Stop further processing
                    }
                }
            }
            else
            {
                // Temporary condition; just log and retry next loop
                Serial.println("Central not subscribed — skipping send.");
            }

            lastTransferTime = millis();
        }
    }

    // If nothing in the buffer, move to IDLE (but only if no error)
    if (queue_count == 0 && current_sensor_state != sensor_state::ERROR_STATE)
    {
        current_sensor_state = sensor_state::IDLE;
    }
}

/**
 * @brief Ensure the BLE stack is initialized and the peripheral is
 * advertising or connected as required.
 *
 * Responsibilities:
 * - Initialize BLE exactly once on boot (`setupBluetooth()`).
 * - Start advertising on disconnect and manage a simple back-off for
 *   repeated advertise failures.
 * - Keep this function lightweight and idempotent: it may be called
 *   frequently from the main loop without side-effects beyond managing
 *   BLE lifecycle state.
 *
 * Non-goals: do not implement application-level packet logic here. That
 * responsibility belongs to `state_TransferPacketBatch()` and related
 * routines. Use `BLE_MANAGEMENT` to ensure the stack is ready.
 */
void state_BLEManagement()
{
    static int bleFailCounter = 0;
    const int MAX_BLE_FAILS = 5;

    // --- Added flag to prevent repeated advertising ---
    static bool isAdvertising = false;

    // Initialize BLE once
    if (!bleInitialized)
    {
        if (setupBluetooth())
        {
            Serial.println("BLE initialized successfully.");
            bleInitialized = true;
            bleFailCounter = 0;

            // Move to IDLE after successful init
            current_sensor_state = sensor_state::IDLE;
        }
        else
        {
            bleFailCounter++;
            Serial.print("BLE initialization failed. Attempt ");
            Serial.println(bleFailCounter);

            if (bleFailCounter >= MAX_BLE_FAILS)
            {
                Serial.println("Too many BLE init failures! Escalating to ERROR_STATE.");
                transitionToErrorState();
            }
            return; // Skip further processing until successful init
        }
    }

    // Reconnection logic
    if (!isCentralConnected())
    {
        // --- Only re-advertise once per disconnect ---
        if (!isAdvertising)
        {
            Serial.println("Central disconnected. Re-advertising BLE service...");
            if (!BLE.advertise())
            {
                bleFailCounter++;
                Serial.print("BLE advertise failed. Attempt ");
                Serial.println(bleFailCounter);

                if (bleFailCounter >= MAX_BLE_FAILS)
                {
                    Serial.println("Too many BLE advertise failures! Escalating to ERROR_STATE.");
                    transitionToErrorState();
                }
            }
            else
            {
                // Successfully started advertising, reset fail counter
                Serial.println("BLE advertising restarted successfully.");
                bleFailCounter = 0;
                isAdvertising = true; // <-- prevent repeated spam
            }
        }
    }
    else
    {
        // Central is connected; reset fail counter and optionally move to IDLE
        bleFailCounter = 0;

        // --- Reset flag when central connects ---
        if (isAdvertising)
        {
            isAdvertising = false;
        }

        if (current_sensor_state == sensor_state::BLE_MANAGEMENT)
        {
            current_sensor_state = sensor_state::IDLE;
            Serial.println("Central connected. Returning to IDLE state.");
        }
    }
}

/**
 * @brief Run diagnostics and attempt recovery for unrecoverable errors.
 *
 * Typical flow:
 * - Inspect `previous_state_to_error_state` and log contextual information.
 * - Attempt low-risk recovery steps (reset BLE, clear transient counters,
 *   persist diagnostics) if applicable.
 * - On successful minimal recovery, return to `IDLE` or the previous
 *   state. If recovery repeatedly fails, remain in `ERROR_STATE` so the
 *   problem can be surfaced to the operator.
 *
 * Note: current implementation prints diagnostics and resets to IDLE.
 * Extend with non-blocking, safe recovery steps as needed.
 */
void state_ErrorState()
{
    Serial.println("=== ERROR_STATE entered ===");

    // Log previous state
    switch (previous_state_to_error_state)
    {
    case sensor_state::IDLE:
        Serial.println("From IDLE: Issue occurred while idle.");
        break;

    case sensor_state::CREATE_AND_BUFFER_PACKET:
        Serial.println(
            "From CREATE_AND_BUFFER_PACKET: Issue occurred while creating/buffering packet.");
        break;

    case sensor_state::TRANSFER_PACKET_BATCH:
        Serial.println(
            "From TRANSFER_PACKET_BATCH: Issue occurred while transferring packet batch.");
        break;

    case sensor_state::BLE_MANAGEMENT:
        Serial.println("From BLE_MANAGEMENT: Issue occurred while managing BLE.");
        break;

    case sensor_state::READ_FLASH_MEMORY:
        Serial.println("From READ_FLASH_MEMORY: Issue occurred while reading flash memory.");
        break;

    default:
        Serial.println("From unknown state.");
        break;
    }

    // Reset state to IDLE
    current_sensor_state = sensor_state::IDLE;
    previous_state_to_error_state = sensor_state::IDLE;

    Serial.println("System recovered: back to IDLE state.");
}
/**
 * @brief Restore state from flash memory (placeholder).
 *
 * Implementation should carefully consider atomicity and power-loss
 * scenarios when reading queued packets or configuration from flash.
 * For now this is a no-op.
 */
void state_ReadFlashMemory() {}

// Placeholder for future batch write functionality
// void state_WriteFlashMemoryBufferBatch() {}
