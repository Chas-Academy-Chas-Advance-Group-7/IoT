/**
 * @file bluetooth_manager.h
 * @brief BLE peripheral management for sensor units.
 *
 * Declares functions and constants for initializing the BLE peripheral,
 * checking central connections, and accessing the BLE characteristic
 * for transmitting `SensorPacket` data.
 *
 * Example usage:
 * @code
 * #include "bluetooth_manager.h"
 *
 * void setup() {
 *     if (!setupBluetooth()) {
 *         Serial.println("BLE initialization failed");
 *     }
 * }
 *
 * void loop() {
 *     if (isCentralConnected()) {
 *         BLECharacteristic &charRef = getSensorCharacteristic();
 *         charRef.writeValue(&packet, sizeof(packet));
 *     }
 * }
 * @endcode
 */

#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include "sensor_package_manager.h"
#include <Arduino.h>
#include <ArduinoBLE.h>

/** UUID of the BLE service used by the sensor */
constexpr char SENSOR_SERVICE_UUID[] = "b7cc8ff2-e60c-4d23-b54f-342371309812";

/** UUID of the BLE characteristic used to transmit sensor packets */
constexpr char SENSOR_CHAR_UUID[] = "59dc9a5f-951f-47c4-a0a3-aac37796805f";

/**
 * @brief Initialize the BLE peripheral for the sensor.
 *
 * Sets device name, adds service and characteristic, and starts advertising.
 *
 * @return true if BLE peripheral was successfully initialized, false otherwise.
 */
bool setupBluetooth();

/**
 * @brief Check if a BLE central device is currently connected.
 *
 * @return true if a central device is connected, false otherwise.
 */
bool isCentralConnected();

/**
 * @brief Get a reference to the sensor BLE characteristic.
 *
 * This allows the caller to write sensor packet data to the characteristic.
 *
 * @return Reference to the BLECharacteristic used for sending sensor packets.
 */
BLECharacteristic &getSensorCharacteristic();

#endif // BLUETOOTH_MANAGER_H
