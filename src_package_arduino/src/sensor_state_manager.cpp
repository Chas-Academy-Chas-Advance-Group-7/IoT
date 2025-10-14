#include "sensor_state_manager.h"

// Define the current state
sensor_state current_sensor_state = sensor_state::IDLE;

// Manage and handle switching between different states
void determineSensorState() {}

// Create a new packet and add it to the buffer
void state_CreateAndBufferPacket()
{
    // Create new packet and add current data
    SensorPacket newPacket = assembleSensorPacket();

    // Try to add the packet to the buffer
    if (!addPacketToBuffer(newPacket))
    {
        // Buffer full or other error
        current_sensor_state = sensor_state::ERROR_STATE;
        Serial.println("Buffer full or error while adding packet!");
    }
    else
    {
        // Added successfully to the buffer
        current_sensor_state = sensor_state::IDLE;
        Serial.println("Packet added to buffer.");
    }
}

// Sends one packet from the buffer via BLE. Uses peek/commit to avoid loss,
// respects a per-packet interval, tracks failures, and sets IDLE or ERROR state.
void state_TransferPacketBatch()
{
    static int failed_transmission_attempts_counter = 0;
    static unsigned long lastTransferTime = 0;
    const unsigned long TRANSFER_INTERVAL = 150; // ms between packets

    // Only run if connected to a central and enough time has passed
    if (isCentralConnected() && queue_count > 0 && millis() - lastTransferTime >= TRANSFER_INTERVAL)
    {
        SensorPacket packet;

        // Peek oldest packet in buffer (non-destructive)
        if (peekPacketFromBuffer(packet))
        {
            BLECharacteristic &charRef = getSensorCharacteristic();
            uint8_t buffer[sizeof(SensorPacket)];
            memcpy(buffer, &packet, sizeof(SensorPacket));

            // Only send if the central has subscribed to notifications
            if (charRef.subscribed())
            {
                bool success = charRef.writeValue(buffer, sizeof(SensorPacket));

                if (success)
                {
                    commitPacketRemoval(); // Remove sent packet
                    failed_transmission_attempts_counter = 0;

                    Serial.print("Sent packet to central: ");
                    Serial.print("Sensor ID: ");
                    Serial.print(packet.sensor_id);
                    Serial.print(", Temp: ");
                    Serial.print(packet.temperature);
                    Serial.print(", Hum: ");
                    Serial.print(packet.humidity);
                    Serial.print(", Seq: ");
                    Serial.println(packet.package_sequence_number);
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

    // Return to idle if no error occurred and buffer is empty
    if (queue_count == 0 && current_sensor_state != sensor_state::ERROR_STATE)
    {
        current_sensor_state = sensor_state::IDLE;
    }
}

// Implement later
void state_UpdateServerId() {}

// Implement later
void state_ErrorState()
{
    Serial.println("Entered ERROR_STATE. Check BLE or buffer.");
}

// Implement later
void state_ReadFlashMemory() {}

// Implement later
// void state_WriteFlashMemoryBufferBatch() {}
