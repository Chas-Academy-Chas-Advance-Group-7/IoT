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

/**
 * @brief Determine and handle transitions between sensor states.
 *
 * Currently a placeholder for future state machine logic.
 */
void determineSensorState() {}

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
        current_sensor_state = sensor_state::ERROR_STATE;
        Serial.println("Buffer full or error while adding packet!");
    }
    else
    {
        current_sensor_state = sensor_state::IDLE;
        Serial.println("Packet added to buffer.");
    }
}

/**
 * @brief Send one packet from the buffer via BLE.
 *
 * Uses peek/commit strategy to avoid data loss, respects a per-packet
 * interval, tracks failed attempts, and updates `current_sensor_state`.
 *
 * @note Only sends packets if a central is connected and subscribed.
 *
 * @code
 * state_TransferPacketBatch(); // Call periodically in main loop or task
 * @endcode
 */
void state_TransferPacketBatch()
{
    static int failed_transmission_attempts_counter = 0;
    static unsigned long lastTransferTime = 0;
    const unsigned long TRANSFER_INTERVAL = 150; // ms between packets

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
                    commitPacketRemoval();
                    failed_transmission_attempts_counter = 0;

                    Serial.print("Sent packet to central: ");
                    Serial.print("Sensor ID: ");
                    Serial.print(packet.sensor_id);
                    Serial.print(", Temp: ");
                    Serial.print(packet.temperature);
                    Serial.print(", Hum: ");
                    Serial.print(packet.humidity);
                }
                else
                {
                    failed_transmission_attempts_counter++;

                    Serial.print("Failed BLE setValue (attempt ");
                    Serial.print(failed_transmission_attempts_counter);
                    Serial.println(")");

                    if (failed_transmission_attempts_counter >= MAX_FAILED_ATTEMPTS)
                    {
                        current_sensor_state = sensor_state::ERROR_STATE;
                        Serial.println("ERROR: BLE transmission failed too many times!");
                    }
                }
            }
            else
            {
                Serial.println("Central not subscribed — skipping send");
            }

            lastTransferTime = millis();
        }
    }

    if (queue_count == 0 && current_sensor_state != sensor_state::ERROR_STATE)
    {
        current_sensor_state = sensor_state::IDLE;
    }
}

/**
 * @brief Handle the ERROR_STATE.
 *
 * Prints diagnostic message and can include recovery logic.
 */
void state_ErrorState()
{
    Serial.println("Entered ERROR_STATE. Check BLE or buffer.");
}

/**
 * @brief Read sensor state from flash memory.
 *
 * Placeholder for future implementation.
 */
void state_ReadFlashMemory() {}

// Placeholder for future batch write functionality
// void state_WriteFlashMemoryBufferBatch() {}
