/**
 * @file bluetooth_manager.cpp
 * @brief BLE peripheral setup and management for sensor units.
 *
 * Provides functions to initialize the BLE peripheral, check central connections,
 * and access the BLE characteristic used to send `SensorPacket` data.
 *
 * This code allows the sensor device to advertise itself and send sensor readings
 * to a connected BLE central (e.g., an ESP32 broker).
 *
 * Example usage:
 * @code
 * if (setupBluetooth()) {
 *     Serial.println("BLE initialized.");
 * }
 *
 * while (1) {
 *     if (isCentralConnected()) {
 *         BLECharacteristic &charRef = getSensorCharacteristic();
 *         charRef.writeValue(&packet, sizeof(packet));
 *     }
 * }
 * @endcode
 */

#include "bluetooth_manager.h"

/** BLE service for the sensor unit */
BLEService sensorService(SENSOR_SERVICE_UUID);

/** BLE characteristic used to transmit sensor packets */
BLECharacteristic sensorChar(SENSOR_CHAR_UUID, BLERead | BLENotify, sizeof(SensorPacket));

/**
 * @brief Initialize the BLE peripheral for the sensor.
 *
 * Sets up the BLE device name, adds the service and characteristic, and starts advertising.
 *
 * @return true if BLE peripheral was successfully initialized, false otherwise.
 *
 * @code
 * if (!setupBluetooth()) {
 *     Serial.println("Failed to initialize BLE");
 * }
 * @endcode
 */
bool setupBluetooth()
{
    if (!BLE.begin())
    {
        Serial.println("Could not setup bluetooth.");
        return false; // failed to setup bluetooth
    }

    // Set the local device name including the sensor ID
    char deviceName[20];
    sprintf(deviceName, "Sensor_%d", sensor_id);
    BLE.setLocalName(deviceName);

    // Setup service and characteristic
    sensorService.addCharacteristic(sensorChar);
    BLE.addService(sensorService);

    BLE.setAdvertisedService(sensorService);
    BLE.advertise();

    return true;
}

/**
 * @brief Check if a BLE central device is currently connected.
 *
 * @return true if a central device is connected, false otherwise.
 *
 * @code
 * if (isCentralConnected()) {
 *     Serial.println("Central device connected");
 * }
 * @endcode
 */
bool isCentralConnected()
{
    return BLE.connected();
}

/**
 * @brief Get a reference to the sensor BLE characteristic.
 *
 * This allows the caller to write sensor packet data to the characteristic.
 *
 * @return Reference to the BLECharacteristic object used for sending sensor packets.
 *
 * @code
 * BLECharacteristic &charRef = getSensorCharacteristic();
 * charRef.writeValue(&packet, sizeof(packet));
 * @endcode
 */
BLECharacteristic &getSensorCharacteristic()
{
    return sensorChar;
}
