#include "sensor_data.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>

parse_result_t parseBLEData(const char *manufacturerData, size_t dataLength,
                            const char *deviceAddress, int8_t rssi)
{
    parse_result_t result;
    result.parse_success = false;
    result.error_message = "Unknown error";

    if (!manufacturerData || dataLength == 0)
    {
        result.error_message = "Empty manufacturer data";
        return result;
    }

    if (dataLength < 8)
    {
        result.error_message = "Manufacturer data too short";
        return result;
    }

    sensor_message_t *sensor = &result.sensor_data;
    memset(sensor, 0, sizeof(sensor_message_t));

    if (deviceAddress)
    {
        strncpy(sensor->device_id, deviceAddress, MAX_DEVICE_ID_LEN - 1);
        sensor->device_id[MAX_DEVICE_ID_LEN - 1] = '\0';
    }

    sensor->rssi = rssi;
    sensor->timestamp = millis();

    const uint8_t *data = (const uint8_t *)manufacturerData;

    if (dataLength >= 2)
    {
        uint16_t company_id = (data[1] << 8) | data[0];

        if (company_id == 0xFFFF)
        {
            if (dataLength >= 8)
            {
                sensor->sensor_type = (sensor_type_t)data[2];

                switch (sensor->sensor_type)
                {
                case SENSOR_TYPE_TEMPERATURE:
                    strncpy(sensor->sensor_name, "Temperature", MAX_SENSOR_NAME_LEN - 1);
                    sensor->value = ((int16_t)((data[4] << 8) | data[3])) / 100.0f;
                    break;
                case SENSOR_TYPE_HUMIDITY:
                    strncpy(sensor->sensor_name, "Humidity", MAX_SENSOR_NAME_LEN - 1);
                    sensor->value = ((int16_t)((data[4] << 8) | data[3])) / 100.0f;
                    break;
                default:
                    sensor->sensor_type = SENSOR_TYPE_UNKNOWN;
                    strncpy(sensor->sensor_name, "Unknown", MAX_SENSOR_NAME_LEN - 1);
                    sensor->value = 0.0f;
                    result.error_message = "Unknown sensor type";
                    return result;
                }

                sensor->is_valid = validateSensorData(sensor);
                if (!sensor->is_valid)
                {
                    result.error_message = "Sensor data validation failed";
                    return result;
                }

                result.parse_success = true;
                result.error_message = nullptr;
            }
            else
            {
                result.error_message = "Insufficient data for sensor reading";
            }
        }
        else
        {
            result.error_message = "Unknown company ID";
        }
    }
    else
    {
        result.error_message = "Invalid manufacturer data format";
    }

    return result;
}

bool validateSensorData(const sensor_message_t *data)
{
    if (!data)
        return false;

    switch (data->sensor_type)
    {
    case SENSOR_TYPE_TEMPERATURE:
        return (data->value >= -40.0f && data->value <= 85.0f);
    case SENSOR_TYPE_HUMIDITY:
        return (data->value >= 0.0f && data->value <= 100.0f);
    default:
        return false;
    }
}

void logSensorReading(const sensor_message_t *data)
{
    if (!data)
    {
        safePrintln("Error: NULL sensor data");
        return;
    }

    safePrintf("=== Sensor Reading ===\n");
    safePrintf("Device: %s\n", data->device_id);
    safePrintf("Type: %s\n", data->sensor_name);
    safePrintf("Value: %.2f", data->value);

    switch (data->sensor_type)
    {
    case SENSOR_TYPE_TEMPERATURE:
        safePrintf(" deg C\n");
        break;
    case SENSOR_TYPE_HUMIDITY:
        safePrintf(" %%\n");
        break;
    default:
        safePrintf("\n");
        break;
    }

    safePrintf("RSSI: %d dBm\n", data->rssi);
    safePrintf("Timestamp: %lu ms\n", data->timestamp);
    safePrintf("Valid: %s\n", data->is_valid ? "Yes" : "No");
    safePrintf("=====================\n");
}

void logParseError(const char *deviceAddress, const char *error)
{
    safePrintf("=== Parse Error ===\n");
    safePrintf("Device: %s\n", deviceAddress ? deviceAddress : "Unknown");
    safePrintf("Error: %s\n", error ? error : "Unknown error");
    safePrintf("==================\n");
}