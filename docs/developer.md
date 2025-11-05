# Developer Guide

This document covers workflows, configuration, testing, and contribution guidelines for developers working on the **IoT Sensor System**.

---

## 1. Build & Upload (PlatformIO)

### Sensor firmware (Arduino Uno R4 WiFi)

```bash
cd src_package_arduino
pio run -e uno_r4_wifi -t upload
```

> Using Arduino IDE: open `src_package_arduino`, select Uno R4 WiFi, and upload.

### Broker firmware (ESP32)

```bash
cd src_broker_esp32
pio run -e esp32_broker -t upload
```

### Running Unit Tests

```bash
cd src_package_arduino
pio test -e native
```

> Unit tests are configured to run on the native environment (host machine) for Arduino code.

---

## 2. Secrets & Configuration

* Copy example secret headers locally and edit with your own credentials:

```bash
cp src_broker_esp32/include/WiFi_secrets_example.h src_broker_esp32/include/WiFi_secrets.h
cp src_broker_esp32/include/backend_server_secrets_example.h src_broker_esp32/include/backend_server_secrets.h
```

* Backend URL and access keys should be defined in `backend_server_secrets.h` and used in:

  * `httptransmission_task.cpp`
  * `backend_task.cpp`

> Do not commit secrets to the repository.

---

## 3. Runtime Architecture & Integration

High-level data flow:

1. Sensor unit (Arduino) reads DHT → creates `SensorPacket` → advertises via BLE.
2. Hub (ESP32) scans BLE → receives packet → adds to `dataQueue`.
3. `Backend_task` converts `SensorPacket` → JSON → pushes to `networkQueue`.
4. `Httptransmission_task` sends JSON → backend via HTTPS.

### Key Files to Inspect

* Sensor BLE & packet creation:

  * `src_package_arduino/src/main.cpp`
  * `sensor_package_manager.h`
  * `bluetooth_manager.h`
* Shared packet layout:

  * `sensor_data.h`
  * `sensor_packet_from_sensor.h`
* Broker runtime & tasks:

  * `src_broker_esp32/src/main.cpp`
  * `src_broker_esp32/src/tasks/*.cpp`

> Ensure `SensorPacket` binary layout is consistent between Sensor Unit and Hub.

---

## 4. Testing, CI & Code Quality

* **Unit tests:** `src_package_arduino/test/`
* **Static analysis:** `clang-tidy` (via PlatformIO config)
* **Formatting:** `clang-format` (repository-local `.clang-format`)
* **Documentation:** generate with Doxygen (`doxygen Doxyfile`)

---

## 5. Developer Scripts & Utilities

* CLI helper scripts:

  * `src_package_arduino/cli.sh` (sensor helpers)
  * `src_broker_esp32/cli.sh` (broker helpers)
* Utility scripts: `tools/convert.py` (compile database / other helper tasks)

---

## 6. Contribution Checklist

Before opening a Pull Request (PR):

1. Run `pio test` for the affected project.
2. Apply `clang-format` and `clang-tidy` to your code.
3. Update Doxygen documentation if public APIs or packet layout changed.
4. Do **not** commit secret header files (`WiFi_secrets.h` or `backend_server_secrets.h`).

---
