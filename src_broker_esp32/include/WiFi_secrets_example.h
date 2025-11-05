/**
 * @file WiFi_secrets_example.h
 * @brief Example Wi-Fi credentials for ESP32 projects.
 *
 * Provides default SSID and password values for compilation and testing.
 * Users should create their own `WiFi_secrets.h` file with actual credentials,
 * which will take priority over this example file.
 *
 * Example usage:
 * @code
 * #include "WiFi_secrets_example.h"
 * WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 * @endcode
 */

#ifndef WIFI_SECRETS_EXAMPLE_H
#define WIFI_SECRETS_EXAMPLE_H

/** Default Wi-Fi network SSID for testing */
#define WIFI_SSID "defaultSSID"

/** Default Wi-Fi network password for testing */
#define WIFI_PASSWORD "defaultPassword"

#endif // WIFI_SECRETS_EXAMPLE_H
