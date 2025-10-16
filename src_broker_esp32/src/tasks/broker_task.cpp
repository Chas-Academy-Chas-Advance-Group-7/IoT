#include "tasks/broker_task.h"
#include "sensor_packet_from_sensor.h"
#include "utils/threadsafe_serial.h"
#include <BLEClient.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>
#include <vector>

extern QueueHandle_t dataQueue;

// UUIDs for your sensor
static BLEUUID SENSOR_SERVICE_UUID("b7cc8ff2-e60c-4d23-b54f-342371309812");
static BLEUUID SENSOR_CHAR_UUID("59dc9a5f-951f-47c4-a0a3-aac37796805f");

// Internal struct for connections
struct SensorConnection
{
    BLEAddress address;
    BLEClient *client;
    BLERemoteCharacteristic *characteristic;
};

static std::vector<SensorConnection> connectedSensors;

// Notification callback: simply forwards raw SensorPacket to the queue
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
    safePrintf("ServerPkgID: %lu\n", (unsigned long)packet.server_package_id);
    safePrintf("Seq #: %u\n", packet.package_sequence_number);
    safePrintf("--------------------\n");

    xQueueSend(dataQueue, &packet, pdMS_TO_TICKS(100));
}

// Connect and subscribe to BLE sensor
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

// Broker task main loop
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
