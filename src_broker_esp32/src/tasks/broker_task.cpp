/**
 * @file broker_task.cpp
 * @brief Broker task for discovering BLE sensors and forwarding their data.
 *
 * This task scans for BLE sensors with a specific service UUID, connects to them,
 * subscribes to notifications, and forwards incoming `SensorPacket`s to the
 * `dataQueue` for processing by the backend task.
 *
 * Thread-safe Serial logging is used through `safePrintf()`.
 *
 * ### Workflow
 * 1. Initialize BLE and start scanning.
 * 2. For each discovered sensor with the correct service UUID:
 *    - Check if already connected.
 *    - Connect and subscribe if new.
 * 3. Notifications from sensors are forwarded directly to `dataQueue`.
 *
 * @note This task runs indefinitely inside a `while(true)` loop.
 */

#include "tasks/broker_task.h"
#include "sensor_packet_from_sensor.h"
#include "utils/threadsafe_serial.h"
#include <BLEClient.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>
#include <vector>

/// Queue handle for forwarding raw sensor packets to backend task.
extern QueueHandle_t dataQueue;

/// UUID of the BLE sensor service to search for.
static BLEUUID SENSOR_SERVICE_UUID("b7cc8ff2-e60c-4d23-b54f-342371309812");

/// UUID of the characteristic containing sensor data.
static BLEUUID SENSOR_CHAR_UUID("59dc9a5f-951f-47c4-a0a3-aac37796805f");

/// Internal struct to track connected BLE sensors
struct SensorConnection
{
    BLEAddress address;                      /**< BLE device address */
    BLEClient *client;                       /**< BLE client object */
    BLERemoteCharacteristic *characteristic; /**< Characteristic used for notifications */
};

/// List of currently connected sensors
static std::vector<SensorConnection> connectedSensors;

/**
 * @brief Callback for BLE characteristic notifications.
 *
 * Converts received raw data into a `SensorPacket` and forwards it to `dataQueue`.
 *
 * @param pCharacteristic Pointer to the BLE characteristic triggering the callback.
 * @param pData Pointer to the raw notification data.
 * @param length Length of the data in bytes.
 * @param isNotify True if this was a notification (vs indication).
 *
 * @note Only packets matching the size of `SensorPacket` are forwarded.
 *
 * @code
 * // Registered internally via:
 * characteristic->registerForNotify(notificationCallback);
 * @endcode
 */
static void notificationCallback(BLERemoteCharacteristic *pCharacteristic, uint8_t *pData,
                                 size_t length, bool isNotify)
{
    if (length != sizeof(SensorPacket))
        return;

    SensorPacket packet;
    memcpy(&packet, pData, sizeof(SensorPacket));

    safePrintf("\n--- Sensor Packet ---\n");
    safePrintf("Sensor ID: %u\n", packet.sensor_id);
    safePrintf("Timestamp: %lu\n", (unsigned long)packet.sensor_timestamp);
    safePrintf("Temperature: %.2f C\n", packet.temperature);
    safePrintf("Humidity: %.2f %%\n", packet.humidity);
    safePrintf("--------------------\n");

    xQueueSend(dataQueue, &packet, pdMS_TO_TICKS(100));
}

/**
 * @brief Connects to a BLE sensor and subscribes to its notifications.
 *
 * @param address BLEAddress of the target sensor.
 * @return True if connection and subscription were successful, false otherwise.
 *
 * @note Adds the new connection to `connectedSensors`.
 *
 * @code
 * BLEAddress addr("12:34:56:78:9A:BC");
 * if (connectToSensor(addr)) { ... }
 * @endcode
 */
static bool connectToSensor(BLEAddress address)
{
    BLEClient *client = BLEDevice::createClient();
    if (!client->connect(address))
    {
        delete client;
        return false;
    }

    BLERemoteService *service = client->getService(SENSOR_SERVICE_UUID);
    if (!service)
    {
        client->disconnect();
        delete client;
        return false;
    }

    BLERemoteCharacteristic *characteristic = service->getCharacteristic(SENSOR_CHAR_UUID);
    if (!characteristic)
    {
        client->disconnect();
        delete client;
        return false;
    }

    characteristic->registerForNotify(notificationCallback);
    connectedSensors.push_back({address, client, characteristic});
    safePrintf("✅ Connected and subscribed to %s\n", address.toString().c_str());
    return true;
}

/**
 * @brief Main broker task function.
 *
 * Scans for BLE sensors, connects to new devices, and handles notifications.
 * Notifications are forwarded to the `dataQueue`.
 *
 * @param parameter Unused task parameter (required by FreeRTOS).
 *
 * @code
 * // Create the broker task:
 * xTaskCreate(brokerTask, "BrokerTask", 8192, NULL, 1, NULL);
 * @endcode
 */
void brokerTask(void *parameter)
{
    safePrintf("Broker task started - initializing BLE\n");
    BLEDevice::init("ESP32-Broker");

    BLEScan *scanner = BLEDevice::getScan();
    scanner->setActiveScan(true);
    scanner->setInterval(1000);
    scanner->setWindow(500);

    while (1)
    {
        safePrintf("Scanning for sensors...\n");
        BLEScanResults results = scanner->start(5);

        for (int i = 0; i < results.getCount(); i++)
        {
            BLEAdvertisedDevice advertisedDevice = results.getDevice(i);
            if (advertisedDevice.haveServiceUUID() &&
                advertisedDevice.isAdvertisingService(SENSOR_SERVICE_UUID))
            {
                BLEAddress addr = advertisedDevice.getAddress();
                bool alreadyConnected = false;

                for (auto &s : connectedSensors)
                {
                    if (s.address.equals(addr))
                    {
                        alreadyConnected = true;
                        break;
                    }
                }

                if (!alreadyConnected)
                {
                    connectToSensor(addr);
                }
            }
        }

        scanner->clearResults();
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
