#ifndef DHT_HANDLER_TEST_HELPERS_H
#define DHT_HANDLER_TEST_HELPERS_H

#ifdef UNIT_TEST

#include <cstdint>

typedef struct
{
    uint8_t sensor_id;
    uint32_t sensor_timestamp;
    float temperature;
    float humidity;
    uint32_t server_package_id;
} SensorPacket;

void setFakeDHTValues(float temp, float hum);
SensorPacket readDHT_Fake(uint8_t sensorId, uint32_t timestamp, uint32_t serverId);

#endif

#endif
