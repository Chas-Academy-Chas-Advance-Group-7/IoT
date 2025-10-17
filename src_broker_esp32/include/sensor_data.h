/**
 * @file sensor_data.h
 * @brief Header for parsing, validating, and logging sensor data from BLE devices.
 *
 * Provides definitions and functions for handling BLE manufacturer data:
 * - `parseBLEData` converts raw BLE data into structured sensor readings.
 * - `validateSensorData` checks readings for plausibility.
 * - `logSensorReading` prints readings to the thread-safe Serial.
 * - `logParseError` prints parsing errors.
 *
 * Constants for maximum string lengths and sensor types are also defined.
 *
 * Example usage:
 * @code
 * const char *rawData = ...;
 * parse_result_t result = parseBLEData(rawData, length, "AA:BB:CC:DD:EE:FF", -65);
 * if (result.parse_success) {
 *     logSensorReading(&result.sensor_data);
 * } else {
 *     logParseError("AA:BB:CC:DD:EE:FF", result.error_message);
 * }
 * @endcode
 */

#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <Arduino.h>

/** Maximum length of a BLE device ID string including null terminator */
#define MAX_DEVICE_ID_LEN 18

/** Maximum length of a sensor name string including null terminator */
#define MAX_SENSOR_NAME_LEN 16

/**
 * @brief Sensor types supported by the system.
 */
typedef enum
{
    SENSOR_TYPE_TEMPERATURE = 1, /**< Temperature sensor */
    SENSOR_TYPE_HUMIDITY = 2,    /**< Humidity sensor */
    SENSOR_TYPE_UNKNOWN = 0      /**< Unknown sensor type */
} sensor_type_t;

/**
 * @brief Structure representing a sensor reading.
 */
typedef struct
{
    char device_id[MAX_DEVICE_ID_LEN];     /**< BLE device ID */
    sensor_type_t sensor_type;             /**< Type of the sensor */
    char sensor_name[MAX_SENSOR_NAME_LEN]; /**< Human-readable name */
    float value;                           /**< Sensor value (Celsius or % humidity) */
    uint32_t timestamp;                    /**< Time of reading in milliseconds */
    int8_t rssi;                           /**< BLE signal strength in dBm */
    bool is_valid;                         /**< True if reading passes validation */
} sensor_message_t;

/**
 * @brief Structure for processed JSON-ready sensor data.
 */
typedef struct
{
    char json[512]; /**< JSON string representation of sensor data */
} processed_data_t;

/**
 * @brief Result of parsing raw BLE manufacturer data.
 */
typedef struct
{
    sensor_message_t sensor_data; /**< Parsed sensor reading */
    bool parse_success;           /**< True if parsing succeeded */
    const char *error_message;    /**< Description of error if parsing failed */
} parse_result_t;

/**
 * @brief Parse BLE manufacturer data into a structured sensor message.
 *
 * @param manufacturerData Pointer to the raw manufacturer data.
 * @param dataLength Length of the manufacturer data.
 * @param deviceAddress Optional BLE device address (used for logging).
 * @param rssi RSSI value of the received packet.
 * @return `parse_result_t` containing the parsed sensor message, success flag, and error message.
 */
parse_result_t parseBLEData(const char *manufacturerData, size_t dataLength,
                            const char *deviceAddress, int8_t rssi);

/**
 * @brief Validate a sensor reading.
 *
 * Checks if the sensor value is within plausible bounds depending on type.
 *
 * @param data Pointer to the sensor message.
 * @return True if valid, false otherwise.
 */
bool validateSensorData(const sensor_message_t *data);

/**
 * @brief Log a sensor reading to thread-safe Serial.
 *
 * @param data Pointer to the sensor message.
 */
void logSensorReading(const sensor_message_t *data);

/**
 * @brief Log a parse error for a given BLE device.
 *
 * @param deviceAddress BLE device address (or NULL if unknown)
 * @param error Error message string
 */
void logParseError(const char *deviceAddress, const char *error);

#endif // SENSOR_DATA_H
