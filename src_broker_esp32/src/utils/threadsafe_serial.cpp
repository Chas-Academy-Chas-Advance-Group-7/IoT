/**
 * @file threadsafe_serial.cpp
 * @brief Thread-safe wrapper functions for Arduino Serial printing.
 *
 * This file provides utility functions to safely print to the Serial interface
 * when using FreeRTOS tasks. A mutex (`serialMutex`) is used to prevent
 * concurrent access to the Serial port from multiple tasks, avoiding message corruption.
 *
 * @note Make sure `serialMutex` is initialized before calling these functions, e.g.:
 * @code
 * serialMutex = xSemaphoreCreateMutex();
 * @endcode
 */

#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <stdarg.h>

/**
 * @brief Thread-safe version of Serial.print() for C-style strings.
 *
 * Acquires the `serialMutex` before printing to ensure exclusive access
 * to the Serial interface. Releases the mutex after printing.
 *
 * @param message Pointer to a null-terminated C string to print.
 *
 * @code
 * safePrint("System starting...");
 * @endcode
 */
void safePrint(const char *message)
{
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        Serial.print(message);
        xSemaphoreGive(serialMutex);
    }
}

/**
 * @brief Thread-safe version of Serial.print() for Arduino String objects.
 *
 * Acquires the `serialMutex` before printing to ensure exclusive access
 * to the Serial interface. Releases the mutex after printing.
 *
 * @param message Reference to a String object to print.
 *
 * @code
 * String msg = "Temperature: " + String(tempValue);
 * safePrint(msg);
 * @endcode
 */
void safePrint(const String &message)
{
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        Serial.print(message);
        xSemaphoreGive(serialMutex);
    }
}

/**
 * @brief Thread-safe version of Serial.println() for C-style strings.
 *
 * Prints the given message followed by a newline, ensuring exclusive access
 * to the Serial interface using `serialMutex`.
 *
 * @param message Pointer to a null-terminated C string to print.
 *
 * @code
 * safePrintln("Initialization complete!");
 * @endcode
 */
void safePrintln(const char *message)
{
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        Serial.println(message);
        xSemaphoreGive(serialMutex);
    }
}

/**
 * @brief Thread-safe version of Serial.println() for Arduino String objects.
 *
 * Prints the given String followed by a newline, ensuring exclusive access
 * to the Serial interface using `serialMutex`.
 *
 * @param message Reference to a String object to print.
 *
 * @code
 * String status = "Motor speed: " + String(speed);
 * safePrintln(status);
 * @endcode
 */
void safePrintln(const String &message)
{
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        Serial.println(message);
        xSemaphoreGive(serialMutex);
    }
}

/**
 * @brief Thread-safe printf-like function for formatted Serial output.
 *
 * Supports formatted printing similar to printf(). Acquires `serialMutex`
 * to ensure that multiple tasks don’t write simultaneously to the Serial port.
 *
 * @param format C-style format string (e.g. "Value: %d\n").
 * @param ...    Variable arguments corresponding to the format specifiers.
 *
 * @note The output buffer is limited to 128 bytes.
 *
 * @code
 * int count = 42;
 * float voltage = 3.3;
 * safePrintf("Count: %d, Voltage: %.2f V\n", count, voltage);
 * @endcode
 */
void safePrintf(const char *format, ...)
{
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        char buffer[128];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Serial.print(buffer);
        xSemaphoreGive(serialMutex);
    }
}
