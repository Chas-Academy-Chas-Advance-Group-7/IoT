#include "tasks/broker_task.h"
#include "sensor_packet_from_sensor.h" // The packed struct definition
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <BLEClient.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>
#include <vector>

extern QueueHandle_t dataQueue;

// UUIDs must match your Arduino (sensor) side exactly!
static BLEUUID SENSOR_SERVICE_UUID("b7cc8ff2-e60c-4d23-b54f-342371309812");
static BLEUUID SENSOR_CHAR_UUID("59dc9a5f-951f-47c4-a0a3-aac37796805f");

struct SensorConnection
{
    BLEAddress address;
    BLEClient *client;
    BLERemoteCharacteristic *characteristic;
};

static std::vector<SensorConnection> connectedSensors;

// Convert SensorPacket → JSON
static String sensorPacketToJson(const SensorPacket &packet)
{
    String json = "{";
    json += "\"sensor_id\":" + String(packet.sensor_id) + ",";
    json += "\"timestamp\":" + String(packet.sensor_timestamp) + ",";
    json += "\"temperature\":" + String(packet.temperature, 2) + ",";
    json += "\"humidity\":" + String(packet.humidity, 2) + ",";
    json += "\"server_package_id\":" + String(packet.server_package_id) + ",";
    json += "\"package_sequence_number\":" + String(packet.package_sequence_number);
    json += "}";
    return json;
}

// Notification callback: handle raw SensorPacket
static void notificationCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData,
                                 size_t length, bool isNotify)
{
    if (length != sizeof(SensorPacket))
    {
        safePrintf("Unexpected packet size: %u (expected %u)\n", (unsigned)length,
                   (unsigned)sizeof(SensorPacket));
        return;
    }

    SensorPacket packet;
    memcpy(&packet, pData, sizeof(SensorPacket));

    // Log received packet
    safePrintf("\n--- Sensor Packet ---\n");
    safePrintf("Sensor ID: %u\n", packet.sensor_id);
    safePrintf("Timestamp: %lu\n", (unsigned long)packet.sensor_timestamp);
    safePrintf("Temperature: %.2f C\n", packet.temperature);
    safePrintf("Humidity: %.2f %%\n", packet.humidity);
    safePrintf("ServerPkgID: %lu\n", (unsigned long)packet.server_package_id);
    safePrintf("Seq #: %u\n", packet.package_sequence_number);
    safePrintf("--------------------\n");

    // Queue for backend
    if (xQueueSend(dataQueue, &packet, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        safePrintf("Failed to enqueue packet from sensor %u\n", packet.sensor_id);
    }
    else
    {
        safePrintf("Packet queued successfully\n");
    }
}

// Connect to sensor and subscribe
static bool connectToSensor(BLEAddress address)
{
    BLEClient *client = BLEDevice::createClient();
    safePrintf("Connecting to sensor %s...\n", address.toString().c_str());

    if (!client->connect(address))
    {
        safePrintf("Failed to connect to %s\n", address.toString().c_str());
        delete client;
        return false;
    }

    BLERemoteService *service = client->getService(SENSOR_SERVICE_UUID);
    if (!service)
    {
        safePrintf("Service not found on %s\n", address.toString().c_str());
        client->disconnect();
        delete client;
        return false;
    }

    BLERemoteCharacteristic *characteristic = service->getCharacteristic(SENSOR_CHAR_UUID);
    if (!characteristic)
    {
        safePrintf("Characteristic not found on %s\n", address.toString().c_str());
        client->disconnect();
        delete client;
        return false;
    }

    // Subscribe for notifications
    if (characteristic->canNotify())
    {
        characteristic->registerForNotify(notificationCallback);
        safePrintf("Subscribed to sensor notifications.\n");
    }
    else
    {
        safePrintf("Characteristic cannot notify on %s\n", address.toString().c_str());
    }

    connectedSensors.push_back({address, client, characteristic});
    safePrintf("Connected and ready to receive from %s\n", address.toString().c_str());
    return true;
}

// Broker task: scan, connect, and handle sensors
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
        vTaskDelay(pdMS_TO_TICKS(10000)); // wait 10s before next scan
    }
}
