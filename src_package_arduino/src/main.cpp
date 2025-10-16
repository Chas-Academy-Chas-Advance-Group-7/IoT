#include "sensor_state_manager.h"
#include <Arduino.h>

void setup()
{
    Serial.begin(9600);
    // initializeDHT();
    setupBluetooth();

    // BLUETOOTH TEST VALUES
    // Pre-fill buffer with a few test packets
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
}

void loop()
{
    // Debug: print connection & subscription status
    // Serial.print("Central connected? ");
    // Serial.println(BLE.connected());

    // Serial.print("Central subscribed? ");
    // Serial.println(getSensorCharacteristic().subscribed());

    // START OF BLUETOOTH TEST CODE
    // --- TEST MODE: wait for central to connect and subscribe ---
    if (BLE.connected() && getSensorCharacteristic().subscribed())
    {
        static unsigned long lastTransferTime = 0;

        // Only send one packet per interval
        if (millis() - lastTransferTime >= 200 && queue_count > 0)
        {
            // Set state to transfer
            current_sensor_state = sensor_state::TRANSFER_PACKET_BATCH;

            // Attempt to send one packet
            state_TransferPacketBatch();

            lastTransferTime = millis();

            // Debug: show how many packets remain
            Serial.print("Buffer count: ");
            Serial.println(queue_count);
        }
    }
    else
    {
        // Central not connected or not subscribed yet
        static bool printed = false;
        if (!printed)
        {
            Serial.println("Waiting for central to connect & subscribe...");
            printed = true;
        }
        delay(500); // small delay to avoid spamming Serial
    }
    // END OF BLUETOOTH TEST CODE

    determineSensorState();

    /*
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

    case sensor_state::UPDATE_SERVER_PACKAGE_ID:
        state_UpdateServerId();
        break;

    case sensor_state::READ_FLASH_MEMORY:
        break;

        // case sensor_state::WRITE_FLASH_MEMORY_BUFFER_BATCH:
        // break;

    case sensor_state::ERROR_STATE:
        state_ErrorState();
        break;
    }
    */
}
