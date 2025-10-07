#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <Arduino.h>

#define MAX_DEVICE_ID_LEN 18
#define MAX_SENSOR_NAME_LEN 16

typedef enum {
    SENSOR_TYPE_TEMPERATURE = 1,
    SENSOR_TYPE_HUMIDITY = 2,
    SENSOR_TYPE_UNKNOWN = 0
} sensor_type_t;

typedef struct {
    char device_id[MAX_DEVICE_ID_LEN];
    sensor_type_t sensor_type;
    char sensor_name[MAX_SENSOR_NAME_LEN];
    float value;
    uint32_t timestamp;
    int8_t rssi;
    bool is_valid;
} sensor_message_t;

typedef struct {
    char json[512];
} processed_data_t;

typedef struct {
    sensor_message_t sensor_data;
    bool parse_success;
    const char* error_message;
} parse_result_t;

parse_result_t parseBLEData(const char* manufacturerData, size_t dataLength, const char* deviceAddress, int8_t rssi);
bool validateSensorData(const sensor_message_t* data);
void logSensorReading(const sensor_message_t* data);
void logParseError(const char* deviceAddress, const char* error);

#endif
