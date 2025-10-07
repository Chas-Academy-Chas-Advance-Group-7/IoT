#include "tasks/broker_task.h"
#include "sensor_data.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <BLEAdvertisedDevice.h>
#include <BLEDevice.h>
#include <BLEScan.h>

extern QueueHandle_t dataQueue;
extern QueueHandle_t networkQueue;

class AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        if (advertisedDevice.haveManufacturerData())
        {
            String mfgDataStr = advertisedDevice.getManufacturerData().c_str();
            const uint8_t *mfgData = (const uint8_t *)mfgDataStr.c_str();
            size_t dataLength = mfgDataStr.length();

            char deviceAddress[18];
            strcpy(deviceAddress, advertisedDevice.getAddress().toString().c_str());
            int8_t rssi = advertisedDevice.getRSSI();

            // Debug: Log ALL BLE data to understand format
            safePrintf("=== BLE Device Found ===\n");
            safePrintf("Device: %s\n", deviceAddress);
            safePrintf("RSSI: %d dBm\n", rssi);
            safePrintf("Manufacturer Data Length: %d bytes\n", dataLength);

            if (dataLength > 0)
            {
                safePrintf("Raw Hex Data: ");
                for (size_t i = 0; i < dataLength; i++)
                {
                    safePrintf("%02X ", mfgData[i]);
                }
                safePrintf("\n");

                // Show company ID if available
                if (dataLength >= 2)
                {
                    uint16_t company_id = (mfgData[1] << 8) | mfgData[0];
                    safePrintf("Company ID: 0x%04X\n", company_id);
                }

                // Show sensor type if available
                if (dataLength >= 3)
                {
                    safePrintf("Sensor Type (byte 2): %d\n", mfgData[2]);
                }

                // Show data bytes
                if (dataLength >= 4)
                {
                    safePrintf("Data bytes: ");
                    for (size_t i = 3; i < dataLength; i++)
                    {
                        safePrintf("%02X ", mfgData[i]);
                    }
                    safePrintf("\n");
                }
            }
            safePrintf("======================\n");

            // Try to parse with current logic
            if (dataLength > 0)
            {
                parse_result_t result =
                    parseBLEData((const char *)mfgData, dataLength, deviceAddress, rssi);

                if (result.parse_success)
                {
                    logSensorReading(&result.sensor_data);

                    if (xQueueSend(dataQueue, &result.sensor_data, pdMS_TO_TICKS(100)) != pdTRUE)
                    {
                        safePrintf("Failed to queue sensor data\n");
                    }
                }
                else
                {
                    safePrintf("Parse failed: %s\n", result.error_message);
                }
            }
        }

        // Also log devices without manufacturer data for completeness
        else
        {
            safePrintf("Device %s has no manufacturer data\n",
                       advertisedDevice.getAddress().toString().c_str());
        }
    }
};

void brokerTask(void *parameter)
{
    safePrintf("Broker task started - initializing BLE scanning\n");

    BLEDevice::init("ESP32-Broker");
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->start(0, nullptr);

    safePrintf("BLE scanning started - listening for sensor data\n");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
