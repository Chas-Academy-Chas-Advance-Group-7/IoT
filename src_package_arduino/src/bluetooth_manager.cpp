#include "bluetooth_manager.h"

// Define BLE service and characteristic at file scope
BLEService sensorService(SENSOR_SERVICE_UUID);
BLECharacteristic sensorChar(SENSOR_CHAR_UUID, BLERead | BLENotify, sizeof(SensorPacket));

// Initialize BLE peripheral
bool setupBluetooth()
{
    if (!BLE.begin())
    {
        Serial.println("Could not setup bluetooth.");
        return false; // failed to setup bluetooth
    }

    // Set the local name to include the sensor ID
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

// Check if a central device is connected
bool isCentralConnected()
{
    return BLE.connected();
}

// Return a reference to the characteristic for sending packets elsewhere
BLECharacteristic &getSensorCharacteristic()
{
    return sensorChar;
}