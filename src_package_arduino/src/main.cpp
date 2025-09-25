#include "sensor_state_manager.h"
#include <Arduino.h>

void setup()
{
    Serial.begin(9600);
    // initializeDHT();
    setupBluetooth();

    // Pre-fill buffer with a few test packets
    for (int i = 0; i < 5; i++)
    {
        addPacketToBuffer(assembleSensorPacket());
    }

    Serial.println("BLE Transfer Test Started");
}

void loop()
{
    // --- TEST MODE: force BLE transfer ---
    current_sensor_state = sensor_state::TRANSFER_PACKET_BATCH;

    // Attempt to send one packet per loop
    state_TransferPacketBatch();

    // Debug: show how many packets remain
    Serial.print("Buffer count: ");
    Serial.println(queue_count);

    // Small delay for Serial & BLE stability
    delay(50);

    /*determineSensorState();

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
    } */

    /*Serial.println("Hello, Arduino!");

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    delay(1000);*/
}
