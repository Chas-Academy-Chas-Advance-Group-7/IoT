# IoT Sensor System

## Overview

This project implements a complete IoT sensor system with **two units**:

1. **Sensor Unit:** Arduino Uno WiFi Rev 4 equipped with a DHT11 sensor, broadcasting temperature and humidity data via **BLE**.  
2. **Central Hub:** ESP32 device that scans for BLE sensors, processes data, and sends it to a backend server via **Wi-Fi HTTPS**.  

The system is designed for reliable, real-time data acquisition and transmission using:

- **BLE notifications** for sensor-to-hub communication  
- **Circular buffer** to prevent data loss  
- **State machine logic** for robust sensor operation  
- **Backend JSON processing** and HTTP transmission  
- **Network monitoring and auto-reconnection**  

---

## Architecture

### 1. Sensor Unit (Arduino Uno WiFi Rev 4)

- **DHT_handler:** Reads temperature & humidity.  
- **Sensor_package_manager:** Creates `SensorPacket` objects containing sensor data.  
- **Bluetooth_manager:** Manages BLE service and characteristic for notifications.  
- **Buffer_manager:** Circular buffer for queued packets.  
- **Sensor_state_manager:** State machine handles creating, buffering, and sending packets.  
- **Main Loop:** Continuously runs the state machine and BLE transmission logic.  

### 2. Central Hub (ESP32)

- **Backend_task:** Processes incoming sensor packets and formats JSON.  
- **Broker_task:** Handles BLE scanning, connections, and subscription to sensor notifications.  
- **Networkstatus_task:** Monitors Wi-Fi connectivity, triggers reconnection if necessary.  
- **Httptransmission_task:** Sends JSON data to a backend server.  
- **Threadsafe Serial Utilities:** Ensures safe logging from multiple FreeRTOS tasks.  

---

## Hardware Requirements

- **Sensor Unit:**  
  - Arduino Uno WiFi Rev 4  
  - DHT11 sensor  
  - BLE capabilities via onboard Wi-Fi/BLE  
- **Central Hub:**  
  - ESP32 development board with BLE and Wi-Fi  
- Optional: additional sensor units  

---

## Software Requirements

- Arduino IDE or PlatformIO  
- **ArduinoBLE** library for ESP32  
- **DHT** sensor library for Arduino  
- FreeRTOS (built into ESP32 Arduino framework)  
- Wi-Fi credentials (`WiFi_secrets.h` for ESP32 hub)  

---

## Setup

### 1. Sensor Unit (Arduino Uno WiFi)

1. Connect the DHT11 sensor to the appropriate digital pin.  
2. Configure the sensor ID in `sensor_package_manager.h`.  
3. Install required libraries (`DHT`).  
4. Upload the firmware to the Arduino Uno WiFi.  

### 2. Central Hub (ESP32)

1. Connect ESP32 to the development environment.  
2. Add Wi-Fi credentials in `WiFi_secrets.h`.  
3. Configure the backend URL in `httptransmission_task.h`.  
4. Upload the firmware to the ESP32.  

---

## Usage

- The **sensor unit** continuously reads temperature and humidity and broadcasts it via BLE.  
- The **ESP32 hub** scans for BLE sensors, subscribes to notifications, buffers incoming packets, and converts them to JSON.  
- JSON data is sent to the backend via HTTP over Wi-Fi.  
- Network connectivity is monitored with automatic reconnection.  
- Serial logs provide debugging and monitoring information.  

---

// ...existing code...

## Quick start — build & flash

- Build and upload sensor firmware (Arduino Uno R4 WiFi):
  - Use PlatformIO in [src_package_arduino/platformio.ini](src_package_arduino/platformio.ini)
  - Example:
    ```
    cd src_package_arduino
    pio run -e uno_r4_wifi -t upload
    ```
- Build and upload broker firmware (ESP32):
  - Use PlatformIO in [src_broker_esp32/platformio.ini](src_broker_esp32/platformio.ini)
  - Example:
    ```
    cd src_broker_esp32
    pio run -e esp32_broker -t upload
    ```

## Secrets & configuration (DO NOT COMMIT)

- Create Wi‑Fi credentials from the template:
  - [src_broker_esp32/include/WiFi_secrets_example.h](src_broker_esp32/include/WiFi_secrets_example.h) -> copy to `WiFi_secrets.h`
- Backend server credentials:
  - Use [src_broker_esp32/include/backend_server_secrets_example.h](src_broker_esp32/include/backend_server_secrets_example.h)
  - The broker will include user file if present: see [`BACKEND_URL`](src_broker_esp32/src/tasks/httptransmission_task.cpp)
  - If present, user file is [src_broker_esp32/include/backend_server_secrets.h](src_broker_esp32/include/backend_server_secrets.h)

## Hardware wiring & pinout

- DHT sensor pin (sensor unit):
  - GPIO pin defined in [src_package_arduino/src/DHT_handler.cpp](src_package_arduino/src/DHT_handler.cpp) as `DHT_PIN`
- BLE peripheral: Arduino advertises service/characteristic UUIDs in
  - [`SENSOR_SERVICE_UUID`](src_package_arduino/include/bluetooth_manager.h)
  - [`SENSOR_CHAR_UUID`](src_package_arduino/include/bluetooth_manager.h)

## Where to change runtime/config values

- Sensor packet assembly and sensor ID:
  - See [`assembleSensorPacket`](src_package_arduino/include/sensor_package_manager.h) and the `sensor_id` variable in the same file: [`sensor_id`](src_package_arduino/include/sensor_package_manager.h)
- BLE control and transmit helpers:
  - Initialize BLE and check state via [`setupBluetooth`](src_package_arduino/include/bluetooth_manager.h), [`isCentralConnected`](src_package_arduino/include/bluetooth_manager.h), and [`getSensorCharacteristic`](src_package_arduino/include/bluetooth_manager.h)
- Backend JSON and aggregation:
  - Processing and JSON layout: [src_broker_esp32/src/tasks/backend_task.cpp](src_broker_esp32/src/tasks/backend_task.cpp) (see `backendTask` / [`backendTask`](src_broker_esp32/include/tasks/backend_task.h))
- HTTP destination & TLS:
  - Backend URL constant and inclusion logic: [`BACKEND_URL`](src_broker_esp32/src/tasks/httptransmission_task.cpp) and [`httpTransmissionTask`](src_broker_esp32/include/tasks/httptransmission_task.h)

## Example JSON payload (what the broker sends)

Derived from [`backendTask`](src_broker_esp32/src/tasks/backend_task.cpp):

```json
{
  "accessKey": "YOUR_ACCESS_KEY",
  "truck_id": 7,
  "sensors": [
    {
      "sensor_id": 1,
      "data": { "temperature": 22.5, "humidity": 48.7 }
    },
    {
      "sensor_id": 2,
      "data": { "temperature": 21.1, "humidity": 50.2 }
    }
  ]
}

---

## Project Structure

## Documentation

Documentation is generated with **Doxygen**. To generate the docs, run:

```bash
doxygen Doxyfile
```

## Developer Quick Reference

This section collects common developer workflows, exact commands, and repository-specific integration notes.

1) Build & upload (PlatformIO)

- Build & upload sensor firmware (Arduino Uno R4 WiFi):

```powershell
cd src_package_arduino
pio run -e uno_r4_wifi -t upload
```

- Build & upload broker firmware (ESP32):

```powershell
cd src_broker_esp32
pio run -e esp32_broker -t upload
```

- Run native unit tests for the sensor project:

```powershell
cd src_package_arduino
pio test -e native
```

2) Secrets & configuration (DO NOT COMMIT)

- The broker expects local secret headers in `src_broker_esp32/include/`.
  - Copy `WiFi_secrets_example.h` -> `WiFi_secrets.h`
  - Copy `backend_server_secrets_example.h` -> `backend_server_secrets.h`

- Backend URL and access keys are referenced in `src_broker_esp32/src/tasks/httptransmission_task.cpp` and `src_broker_esp32/src/tasks/backend_task.cpp`.

3) Runtime architecture & key integration points

- High level flow: Sensor unit (Arduino) reads DHT → assembles `SensorPacket` → advertises via BLE notifications. Broker (ESP32) scans BLE → subscribes → writes `SensorPacket` into `dataQueue` → `backend_task` converts to JSON and pushes to `networkQueue` → `httptransmission_task` sends to backend.

- Files to inspect for each part:
  - Sensor BLE & packet creation: `src_package_arduino/src/main.cpp`, `src_package_arduino/include/sensor_package_manager.h`, `src_package_arduino/include/bluetooth_manager.h`.
  - Shared packet layout: `src_broker_esp32/include/sensor_data.h` and `src_broker_esp32/include/sensor_packet_from_sensor.h`.
  - Broker runtime wiring and globals (queues, event group, mutex): `src_broker_esp32/src/main.cpp` (see `dataQueue`, `networkQueue`, `networkEventGroup`, `serialMutex`).
  - Broker tasks: `src_broker_esp32/src/tasks/*.cpp` (`broker_task.cpp`, `backend_task.cpp`, `httptransmission_task.cpp`, `networkstatus_task.cpp`).

- Important compatibility rule: keep the binary layout of `SensorPacket` compatible between the sensor and broker projects. If you add/remove fields, update `sensor_data.h` and the sensor assembly code (`sensor_package_manager`) and the JSON mapping (`backend_task.cpp`).

4) JSON / backend notes

- JSON formatting happens in `src_broker_esp32/src/tasks/backend_task.cpp` — verify structure matches backend expectations before changing packet fields.
- The HTTP transmit task is `src_broker_esp32/src/tasks/httptransmission_task.cpp`. Use `backend_server_secrets.h` locally to avoid hardcoding secrets in source.

5) Testing, CI and code quality

- Unit tests: `src_package_arduino/test/` (native env). Run `pio test -e native` from `src_package_arduino`.
- Static analysis: PlatformIO `check_tool = clangtidy` is configured in both `platformio.ini` files. New C++ code should be clang-tidy clean where practical.
- Formatting: repository uses clang-format and project-local `.clang-format` settings. Run formatting before opening PRs.

6) Developer scripts & helpers

- CLI helper scripts:
  - `src_package_arduino/cli.sh` — sensor helper script
  - `src_broker_esp32/cli.sh` — broker helper script
- Utility: `tools/convert.py` (helper for compile-db/other conversions) — document and use as needed.

7) Documentation generation

- Doxygen: `doxygen Doxyfile` will generate HTML into the `docs` directory (see `Doxyfile` for options and exclusions).

8) Troubleshooting & common pitfalls

- Serial logging: multiple FreeRTOS tasks write to Serial on the broker; use `utils/threadsafe_serial.*` and the global `serialMutex` for safe prints (see `src_broker_esp32/src/utils/threadsafe_serial.cpp`).
- BLE subscription issues: when the broker isn't receiving packets, verify the BLE characteristic UUIDs in `src_package_arduino/include/bluetooth_manager.h` match the UUIDs the broker subscribes to. Also confirm the sensor is advertising and BLE central connection is established.
- Packet mismatch: if JSON values look wrong, check binary packet struct layout and endianness in `sensor_data.h` and the sensor's packet assembly (`sensor_package_manager`).
- Queue sizes: `dataQueue` and `networkQueue` default to small depths in `src_broker_esp32/src/main.cpp` (10). Increase if you see drops under load.

9) Contribution checklist (before opening a PR)

- Update code and run `pio test` for the affected project (use `env:native` for unit-tests where available).
- Run clang-format and clang-tidy locally (PlatformIO config shows how clang-tidy runs in the build pipeline).
- Update `Doxyfile` doc comments if public APIs/packet layout changes.
- Do NOT commit `WiFi_secrets.h` or `backend_server_secrets.h`. Add them to local config only.

If you want, I can turn any of the above bullets into step-by-step tasks or add CI snippets that run PlatformIO builds and clang-tidy.

