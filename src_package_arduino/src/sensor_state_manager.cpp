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
void state_TransferPacketBatch() {}

// Implement later
void state_UpdateServerId() {}

// Implement later
void state_ErrorState() {}

// Implement later
void state_ReadFlashMemory() {}

// Implement later
// void state_WriteFlashMemoryBufferBatch() {}