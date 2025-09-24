#include "sensor_state_manager.h"

// Define the current state
sensor_state current_sensor_state = sensor_state::IDLE;

// Implement later
// Manage and handle switching between different states
void determineSensorState() {}

// Create a new packets and adds it to the buffer
void state_CreateAndBufferPacket()
{
    // Create new packet and add current data
    SensorPacket newPacket = assembleSensorPacket();

    // Try to add the packet to the buffer
    if (!addPacketToBuffer(newPacket))
    {
        // Buffer full or other error
        current_sensor_state = sensor_state::ERROR_STATE;
    }

    else
    {
        // Added succesfully to the buffer
        current_sensor_state = sensor_state::IDLE;
    }
}

// Implement later
void state_TransferPacketBatch()
{
    static int failed_transmission_attempts_counter = 0;
    static unsigned long lastTransferTime = 0;
    const unsigned long TRANSFER_INTERVAL = 10;

    // Connected to central unit and there's things in the buffer
    while (isCentralConnected() && queue_count > 0)
    {
        // only run in accordance with transfer interval
        if (millis() - lastTransferTime < TRANSFER_INTERVAL)
        {
            return;
        }

        SensorPacket packet;

        // Copy oldest value from buffer
        if (peekPacketFromBuffer(packet))
        {
            BLECharacteristic &charRef = getSensorCharacteristic();
            uint8_t buffer[sizeof(SensorPacket)];
            memcpy(buffer, &packet, sizeof(SensorPacket));

            if (charRef.writeValue(buffer, sizeof(SensorPacket)))
            {
                commitPacketRemoval(); // Succeeded: erase packet from buffer
                failed_transmission_attempts_counter = 0;
            }

            else
            {
                failed_transmission_attempts_counter++;

                // Debug: show current fail count
                Serial.print("Failed attempts: ");
                Serial.println(failed_transmission_attempts_counter);

                if (failed_transmission_attempts_counter > MAX_FAILED_ATTEMPTS)
                {
                    current_sensor_state = sensor_state::ERROR_STATE;
                    Serial.println("ERROR: failed BLE transmission repeatably.");
                }

                break; // failed: save packet for later attempt
            }
        }

        // Updates for next interval
        lastTransferTime = millis();
    }

    // returns to idle if no error occured and buffer is empty
    if (queue_count == 0 && current_sensor_state != sensor_state::ERROR_STATE)
    {
        current_sensor_state = sensor_state::IDLE;
    }
}

// Implement later
void state_UpdateServerId() {}

// Implement later
void state_ErrorState() {}

// Implement later
void state_ReadFlashMemory() {}

// Implement later
// void state_WriteFlashMemoryBufferBatch() {}