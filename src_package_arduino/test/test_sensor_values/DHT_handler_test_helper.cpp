#ifdef UNIT_TEST
#include "DHT_handler_test_helpers.h"

static float test_temperature = 22.3f;
static float test_humidity = 74.6f;

void setFakeDHTValues(float temp, float hum)
{
    test_temperature = temp;
    test_humidity = hum;
}

SensorPacket readDHT_Fake(uint8_t sensorId, uint32_t timestamp, uint32_t serverId)
{
    SensorPacket packet;
    packet.sensor_id = sensorId;
    packet.sensor_timestamp = timestamp;
    packet.server_package_id = serverId;
    packet.temperature = test_temperature;
    packet.humidity = test_humidity;
    return packet;
}

#endif
