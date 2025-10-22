/**
 * @file main.cpp
 * @brief Main entry point for the sensor firmware.
 *
 * Initializes the sensor system, including DHT sensor and BLE,
 * and runs the main loop to manage the sensor state machine,
 * packet creation, and BLE transfer.
 *
 * Example usage:
 * @code
 * // Main automatically calls determineSensorState() and executes the appropriate state functions.
 * @endcode
 */

#include "sensor_state_manager.h"
#include <Arduino.h>

/**
 * @brief Arduino setup function.
 *
 * Initializes serial communication, DHT sensor, and BLE peripheral.
 * Optionally pre-fills the circular buffer for BLE transfer testing.
 */
void setup()
{
    Serial.begin(9600);

    // Initialize DHT sensor
    initializeDHT();

    // Initialize BLE peripheral
    setupBluetooth();

    /*
    // BLUETOOTH TEST VALUES
    // Pre-fill buffer with test packets
    for (int i = 0; i < 5; i++)
    {
        SensorPacket packet;
        packet.sensor_id = i + 1;
        packet.sensor_timestamp = millis();
        packet.temperature = 20.0 + i; // example temperatures
        packet.humidity = 40.0 + i;    // example humidity
        packet.server_package_id = 100;
        packet.package_sequence_number = i + 1;

        addPacketToBuffer(packet);
    }

    Serial.println("BLE Transfer Test Started");
    */
}

/**
 * @brief Arduino main loop function.
 *
 * Continuously:
 * - Updates the sensor state machine
 * - Executes the logic corresponding to the current state
 * - Handles packet creation, BLE transfer, server updates, or error handling
 *
 * @note Includes commented-out BLE test/debug code that waits for central connection
 *       and subscription, then attempts to transfer buffered packets.
 */
void loop()
{
    // Optional debug prints
    // Serial.print("Central connected? "); Serial.println(BLE.connected());
    // Serial.print("Central subscribed? "); Serial.println(getSensorCharacteristic().subscribed());

    /*
    // START OF BLUETOOTH TEST CODE
    if (BLE.connected() && getSensorCharacteristic().subscribed())
    {
        static unsigned long lastTransferTime = 0;

        if (millis() - lastTransferTime >= 200 && queue_count > 0)
        {
            // Set state to transfer packet batch
            current_sensor_state = sensor_state::TRANSFER_PACKET_BATCH;

            // Attempt to send one packet
            state_TransferPacketBatch();

            lastTransferTime = millis();

            Serial.print("Buffer count: ");
            Serial.println(queue_count);
        }
    }
    else
    {
        static bool printed = false;
        if (!printed)
        {
            Serial.println("Waiting for central to connect & subscribe...");
            printed = true;
        }
        delay(500); // small delay to avoid spamming Serial
    }
    // END OF BLUETOOTH TEST CODE
    */

    // Update state machine
    determineSensorState();

    // Execute logic for the current sensor state
    switch (current_sensor_state)
    {
    case sensor_state::IDLE:
        break;

    case sensor_state::CREATE_AND_BUFFER_PACKET:
        state_CreateAndBufferPacket();
        break;

    case sensor_state::TRANSFER_PACKET_BATCH:
        state_TransferPacketBatch();
        break;

    case sensor_state::READ_FLASH_MEMORY:
        break;

        // case sensor_state::WRITE_FLASH_MEMORY_BUFFER_BATCH:
        // break;

    case sensor_state::ERROR_STATE:
        state_ErrorState();
        break;
    }
}
