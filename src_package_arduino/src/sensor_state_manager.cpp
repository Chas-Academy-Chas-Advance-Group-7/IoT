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
 * @brief Determine and handle transitions between sensor states.
 *
 * Call periodically in the main loop or task.
 */
void determineSensorState()
{
    // Do nothing if in error state — error handler manages recovery
    if (current_sensor_state == sensor_state::ERROR_STATE)
    {
        return;
    }

    // --- 1. BLE management priority ---
    // Check if BLE is initialized or central is disconnected
    if (!bleInitialized || !isCentralConnected())
    {
        current_sensor_state = sensor_state::BLE_MANAGEMENT;
        return;
    }

    // --- 2. Packet transfer priority ---
    // Only try sending if there are packets in the buffer
    if (millis() - lastTransferCheck >= TRANSFER_PERIOD && queue_count > 0)
    {
        lastTransferCheck = millis();
        current_sensor_state = sensor_state::TRANSFER_PACKET_BATCH;
        return;
    }

    // --- 3. Sensor packet creation timing ---
    if (millis() - lastPacketTime >= PACKET_INTERVAL)
    {
        lastPacketTime = millis();
        current_sensor_state = sensor_state::CREATE_AND_BUFFER_PACKET;
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
 * @brief Transition to the ERROR_STATE.
 *
 * Call this function to handle error transitions.
 * It updates the previous state and sets the current state to ERROR_STATE.
 */
void transitionToErrorState()
{
    previous_state_to_error_state = current_sensor_state;
    current_sensor_state = sensor_state::ERROR_STATE;
}

/**
 * @brief Create a new SensorPacket and add it to the circular buffer.
 *
 * Updates the `current_sensor_state` depending on success:
 * - IDLE if packet added successfully
 * - ERROR_STATE if buffer is full or failed to add
 *
 * @code
 * state_CreateAndBufferPacket();
 * @endcode
 */
void state_CreateAndBufferPacket()
{
    SensorPacket newPacket = assembleSensorPacket();

    if (!addPacketToBuffer(newPacket))
    {
        Serial.println("Buffer full or error while adding packet!");
        transitionToErrorState();
    }
    else
    {
        Serial.println("Packet added to buffer.");
        current_sensor_state = sensor_state::IDLE;
    }
}

/**
 * @brief Transfer a batch of packets from the buffer via BLE.
 *
 * Attempts to send packets if a central device is connected and subscribed.
 * On repeated transmission failures, transitions to ERROR_STATE.
 *
 * @code
 * state_TransferPacketBatch();
 * @endcode
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
 * @brief Manage BLE connections and advertising.
 *
 * Initializes BLE if not already done, handles reconnections,
 * and manages advertising state.
 *
 * @code
 * state_BLEManagement();
 * @endcode
 */
void state_BLEManagement()
{
    static int bleFailCounter = 0;
    const int MAX_BLE_FAILS = 5;

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
        }
    }
    else
    {
        // Central is connected; reset fail counter and optionally move to IDLE
        bleFailCounter = 0;
        if (current_sensor_state == sensor_state::BLE_MANAGEMENT)
        {
            current_sensor_state = sensor_state::IDLE;
            Serial.println("Central connected. Returning to IDLE state.");
        }
    }
}

/**
 * @brief Handle the ERROR_STATE.
 *
 * Prints diagnostic message and can include recovery logic.
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
 * @brief Read sensor state from flash memory.
 *
 * Placeholder for future implementation.
 */
void state_ReadFlashMemory() {}

// Placeholder for future batch write functionality
// void state_WriteFlashMemoryBufferBatch() {}
