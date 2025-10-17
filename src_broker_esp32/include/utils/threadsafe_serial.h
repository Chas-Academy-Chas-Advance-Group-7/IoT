/**
 * @file threadsafe_serial.h
 * @brief Thread-safe Serial printing utilities for Arduino with FreeRTOS.
 *
 * Provides safe wrapper functions around the Arduino `Serial` object that can be
 * used across multiple FreeRTOS tasks. A global semaphore (`serialMutex`) is used
 * to synchronize access and prevent concurrent writes to the Serial interface.
 *
 * Example:
 * @code
 * #include "utils/threadsafe_serial.h"
 *
 * void setup() {
 *     Serial.begin(115200);
 *     serialMutex = xSemaphoreCreateMutex();
 * }
 *
 * void loop() {
 *     safePrintln("Hello from loop!");
 *     vTaskDelay(pdMS_TO_TICKS(1000));
 * }
 * @endcode
 */

#ifndef THREADSAFE_SERIAL_H
#define THREADSAFE_SERIAL_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/**
 * @brief Global semaphore used to ensure thread-safe Serial access.
 *
 * Must be created before any safe print function is called:
 * @code
 * serialMutex = xSemaphoreCreateMutex();
 * @endcode
 */
extern SemaphoreHandle_t serialMutex;

/**
 * @brief Thread-safe print for C-style strings.
 * @param message Null-terminated C string to print.
 *
 * @code
 * safePrint("Booting...");
 * @endcode
 */
void safePrint(const char *message);

/**
 * @brief Thread-safe print for Arduino String objects.
 * @param message String object to print.
 *
 * @code
 * String info = "Voltage: " + String(voltage);
 * safePrint(info);
 * @endcode
 */
void safePrint(const String &message);

/**
 * @brief Thread-safe println for C-style strings.
 * @param message Null-terminated C string to print, followed by newline.
 *
 * @code
 * safePrintln("System ready!");
 * @endcode
 */
void safePrintln(const char *message);

/**
 * @brief Thread-safe println for Arduino String objects.
 * @param message String object to print, followed by newline.
 *
 * @code
 * safePrintln("Task completed!");
 * @endcode
 */
void safePrintln(const String &message);

/**
 * @brief Thread-safe printf-style formatted print.
 * @param format Format string (like printf).
 * @param ... Arguments matching the format string.
 *
 * @note Output buffer limited to 128 bytes.
 *
 * @code
 * safePrintf("Sensor value: %d\n", sensorValue);
 * @endcode
 */
void safePrintf(const char *format, ...);

/**
 * @brief Template version of safePrint for any printable type.
 * @tparam T Any type supported by Serial.print().
 * @param value The value to print.
 *
 * @code
 * safePrint(3.1415);
 * safePrint(true);
 * safePrint("Hello!");
 * @endcode
 */
template <typename T> void safePrint(const T &value)
{
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        Serial.print(value);
        xSemaphoreGive(serialMutex);
    }
}

/**
 * @brief Template version of safePrintln for any printable type.
 * @tparam T Any type supported by Serial.println().
 * @param value The value to print followed by a newline.
 *
 * @code
 * safePrintln(42);
 * safePrintln("All done!");
 * @endcode
 */
template <typename T> void safePrintln(const T &value)
{
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        Serial.println(value);
        xSemaphoreGive(serialMutex);
    }
}

#endif // THREADSAFE_SERIAL_H
