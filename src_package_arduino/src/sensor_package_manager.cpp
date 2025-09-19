#include "sensor_package_manager.h"

// Sensor variables
uint8_t sensor_id = 0;
float temperature = 0.0;
float humidity = 0.0;
uint32_t server_package_id = 0;
uint16_t package_sequence_number = 0;

// Assemble a SensorPacket using current sensor data
SensorPacket assembleSensorPacket()
{
    SensorPacket packet;

    packet.sensor_id = sensor_id;
    packet.sensor_timestamp = getTimestamp();
    readDHT(packet);
    packet.server_package_id = server_package_id;
    packet.package_sequence_number = ++package_sequence_number;

    return packet;
}

// currently not implemented
uint32_t getTimestamp() {}