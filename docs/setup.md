# Setup & Configuration

This document describes the hardware, software and step-by-step setup required to run the **IoT Sensor System** (Sensor Unit: Arduino Uno WiFi Rev 4; Central Hub: ESP32).

> **Warning — secrets:** do **not** commit `WiFi_secrets.h` or `backend_server_secrets.h` to version control. Keep them local and make sure they are added if they somehow are missing to [.gitignore](../src_broker_esp32/.gitignore).

---

## Hardware Requirements

### Sensor Unit

* Arduino Uno WiFi Rev 4
* DHT11 sensor (or compatible DHTxx **NOTE:** May require additional code changes not covered) 
* Wires / breadboard / connectors

### Central Hub

* ESP32 development board (with BLE and Wi‑Fi support)
* USB cable for programming

**Optional**: additional sensor units for multi-node testing.

---

## Software Requirements

* **PlatformIO** (recommended) or Arduino IDE
* Required libraries (install via PlatformIO `platformio.ini` or Arduino Library Manager):

  * `DHT` (sensor handling)
  * `ArduinoBLE` (for BLE on sensor or where appropriate)
* ESP32 Arduino core (for the broker project)
* FreeRTOS is included in the ESP32 Arduino framework
* Doxygen (optional) for generating API docs (`doxygen Doxyfile`)

---

## Local Secrets & Configuration (DO NOT COMMIT)

The broker expects local header files for secrets. Copy the examples and edit them with your values:

```bash
cp src_broker_esp32/include/WiFi_secrets_example.h src_broker_esp32/include/WiFi_secrets.h
cp src_broker_esp32/include/backend_server_secrets_example.h src_broker_esp32/include/backend_server_secrets.h
```

* `WiFi_secrets.h` should contain your SSID and password strings.
* `backend_server_secrets.h` may contain `BACKEND_URL`, access keys, or other auth details depending on your setup.

When running in CI or production, inject secrets via environment variables or a secure vault; do **not** store them in the repository.

---

## Build & Upload (Quick Start)

### Sensor firmware (Arduino Uno R4 WiFi)

```bash
cd src_package_arduino
pio run -e uno_r4_wifi -t upload
```

> If using Arduino IDE: open the `src_package_arduino` project, select the Uno R4 WiFi board and upload.

### Broker firmware (ESP32)

```bash
cd src_broker_esp32
pio run -e esp32_broker -t upload
```

---

## Sensor Unit — Detailed Setup

1. **Connect the DHT11 sensor**

   * Connect VCC to 5V (or 3.3V depending on sensor variant and board wiring), GND to GND, and DATA to the digital pin defined as `DHT_PIN` in the source: [`src_package_arduino/src/DHT_handler.cpp`](../src_package_arduino/src/DHT_handler.cpp). 
   * If using pull-up resistor, follow the DHT11 wiring recommendations (typically 4.7k–10kΩ between VCC and DATA).
   
   **NOTE:** The `DHT_PIN` is set to pin `2` as a default.

2. **Sensor ID and packet settings**

   * Edit `sensor_id` and any packet-related constants in:

     * [`src_package_arduino/include/sensor_package_manager.cpp`](../src_package_arduino/src/sensor_package_manager.cpp)
   * Verify `assembleSensorPacket` behavior in the same source if you change fields.

   **NOTE:** The `sensor_id` is set to `0` as a default.

3. **Libraries**

   * Ensure `DHT` is installed and included in `platformio.ini` or Arduino's library manager.

4. **Upload firmware**

   * Use PlatformIO or Arduino IDE to flash the sensor code. After upload, the Arduino will begin advertising the sensor BLE service (see BLE section in [`bluetooth_manager.h`](../src_package_arduino/include/bluetooth_manager.h)).

---

## Central Hub — Detailed Setup (ESP32)

1. **Add Wi‑Fi credentials**

   * Create `src_broker_esp32/include/WiFi_secrets.h` from the example and populate with your SSID/password.

2. **Configure backend URL & secrets**

   * If you have a backend endpoint, either set the `BACKEND_URL` in `src_broker_esp32/include/backend_server_secrets.h` or ensure the constant in `src_broker_esp32/src/tasks/httptransmission_task.cpp` points to the correct URL.

3. **Adjust queue sizes & runtime params** (optional)

   * Default queue depths (e.g. `dataQueue`, `networkQueue`) are defined in [`src_broker_esp32/src/main.cpp`](../src_broker_esp32/src/main.cpp). Increase them if you expect bursts of data.

4. **Upload firmware**

   * Use PlatformIO to build and flash the ESP32 broker firmware.

5. **Verify runtime logs**

   * Open serial monitor (baud rate as defined in [`src_broker_esp32/src/main.cpp`](../src_broker_esp32/src/main.cpp)) and verify that the broker scans for BLE advertisements and connects to sensors.

---

## Wiring & Pinout References

* **DHT sensor pin:** the pin used by the sensor is defined in [`src_package_arduino/src/DHT_handler.cpp`](../src_package_arduino/src/DHT_handler.cpp) as `DHT_PIN`.
* **BLE UUIDs:** the sensor advertises the service/characteristic defined by `SENSOR_SERVICE_UUID` and `SENSOR_CHAR_UUID` in [`src_package_arduino/include/bluetooth_manager.h`](../src_package_arduino/include/bluetooth_manager.h).

Refer to the source files for exact symbol names and pin numbers before wiring.

---

## Notes & Best Practices

* **Powering the boards:** use a stable 5V USB supply when programming. For battery operation, follow recommended regulator/step-up approaches for each board (see project notes / hardware docs).
* **Secrets handling:** add the secrets headers to `.gitignore`.
* **Testing:** verify the backend endpoint with `curl` before performing full integration tests.

---