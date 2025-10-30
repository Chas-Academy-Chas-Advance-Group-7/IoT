/**
 * @file bluetooth_manager.cpp
 * @brief BLE peripheral setup and management for sensor units.
 *
 * Implementation notes and contract:
 * - The characteristic `sensorChar` carries raw binary `SensorPacket`
 *   payloads (see `sensor_package_manager.h`). The central (broker)
 *   must parse the incoming bytes according to that packed layout.
 * - Before attempting to write notifications check `charRef.subscribed()`;
 *   if the central is not subscribed, do not attempt to send payloads.
 * - Keep writes rate-limited (see `TRANSFER_INTERVAL` in state manager).
 * - The code uses `BLE.begin()` and `BLE.advertise()` from ArduinoBLE; some
 *   boards may behave differently with advertising calls — guard against
 *   repeated advertise attempts (the state manager uses an `isAdvertising`
 *   flag for this purpose).
 *
 * Example usage (non-blocking notification):
 * @code
 * if (isCentralConnected()) {
 *     BLECharacteristic &charRef = getSensorCharacteristic();
 *     if (charRef.subscribed()) {
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
