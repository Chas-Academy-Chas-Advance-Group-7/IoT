# IoT Sensor System

This document provides an overview of the **IoT Sensor System**, which consists of a **Sensor Unit** (Arduino Uno WiFi Rev 4) and a **Central Hub** (ESP32). It serves as the main entry point for developers and users.

---

## Overview

The system implements two primary units:

1. **Sensor Unit**: Reads temperature and humidity using a DHT11 sensor and broadcasts data via **BLE**.
2. **Central Hub**: Scans for BLE sensors, processes incoming data, and sends JSON payloads to a backend server over **Wi-Fi HTTPS**.

The system ensures reliable, real-time data acquisition using:

* BLE notifications for sensor-to-hub communication
* Circular buffer to prevent data loss
* State machine logic for robust sensor operation
* Backend JSON formatting and HTTP transmission
* Network monitoring with auto-reconnection

---

## Documentation

For detailed information, refer to the following Markdown files:

* [Setup & Configuration](setup.md) — hardware, software, wiring, secrets, and build instructions
* [System Architecture](architecture.md) — components, tasks, and data flow
* [Developer Guide](developer.md) — developer workflows, testing, and contribution guidelines
* [Backend & JSON Format](backend.md) — backend communication, payload structure, and reliability notes
* [Troubleshooting & Common Pitfalls](troubleshooting.md) — debugging tips and runtime issues

Full API documentation generated with **Doxygen** is also available online:  
[GitHub Pages Doxygen Docs](https://chas-academy-chas-advance-group-7.github.io/IoT/)

---

## Quick Start

### 1. Configure Secrets

Copy the example secret headers and configure with your credentials:

```bash
cp src_broker_esp32/include/WiFi_secrets_example.h src_broker_esp32/include/WiFi_secrets.h
cp src_broker_esp32/include/backend_server_secrets_example.h src_broker_esp32/include/backend_server_secrets.h
```

### 2. Build & Upload Sensor Firmware (Arduino Uno R4 WiFi)

```bash
cd src_package_arduino
pio run -e uno_r4_wifi -t upload
```

> Arduino IDE users can open the project, select the Uno R4 WiFi board, and upload.

### 3. Build & Upload Broker Firmware (ESP32)

```bash
cd src_broker_esp32
pio run -e esp32_broker -t upload
```

### 4. Verify Operation

* Monitor serial output to ensure the sensor is advertising and the hub is connecting via BLE.
* Check that packets are correctly converted to JSON and transmitted to the backend.

---

## Usage

* Sensor unit continuously reads temperature and humidity and broadcasts via BLE.
* ESP32 hub scans for BLE sensors, subscribes to notifications, buffers packets, and converts them to JSON for backend transmission.
* Network connectivity is monitored automatically.
* Serial logs are available for debugging and monitoring.

---

## Project Structure

* `src_package_arduino/` — Sensor firmware source code
* `src_broker_esp32/` — Broker firmware source code
* `docs/` — Detailed Markdown documentation
* `Doxyfile` — Doxygen configuration for generating developer docs

Generate Doxygen documentation:

```bash
doxygen Doxyfile
```

---

## Notes

* Do not commit `WiFi_secrets.h` or `backend_server_secrets.h` to version control.
* Verify BLE UUIDs match between the sensor and hub.
* Ensure consistent `SensorPacket` structure between Arduino and ESP32.
* Adjust queue sizes in ESP32 if packets are dropped under high load.

This README serves as a concise introduction and guide, directing developers and users to the detailed documentation in the `docs/` folder.
