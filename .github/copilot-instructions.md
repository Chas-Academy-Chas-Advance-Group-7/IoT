## Copilot / AI agent instructions — IoT firmware (Sensor Unit + Broker)

Goal: help an AI coding agent be immediately productive in this repo (two PlatformIO firmware projects: sensor unit and ESP32 broker).

Key facts (big picture)
- This repo contains two firmware projects: `src_package_arduino` (Arduino Uno R4 WiFi sensor unit) and `src_broker_esp32` (ESP32 central hub / broker).
- Sensor unit reads DHT data, packages into SensorPacket, and advertises via BLE notifications. See `src_package_arduino/src/main.cpp` and `include/*_handler.h`.
- Broker scans BLE, enqueues raw SensorPacket into `dataQueue`, backend task converts to JSON and `networkQueue` feeds the HTTP transmission task. See `src_broker_esp32/src/main.cpp` and `src_broker_esp32/src/tasks/*.cpp`.

Build, flash & test (exact commands)
- Use PlatformIO at each project root. Key env names:
  - Sensor (Arduino): default env `uno_r4_wifi` in `src_package_arduino/platformio.ini`.
    - Example: `cd src_package_arduino` then `pio run -e uno_r4_wifi -t upload`.
  - Broker (ESP32): env `esp32_broker` in `src_broker_esp32/platformio.ini`.
    - Example: `cd src_broker_esp32` then `pio run -e esp32_broker -t upload`.
- There is a `native` env in the sensor project for unit-testing (`env:native`). Tests are built with `-DUNIT_TEST`.

Secrets & configuration (do not commit)
- Wi‑Fi and backend secrets are provided as example headers in `src_broker_esp32/include/`:
  - Copy `WiFi_secrets_example.h` -> `WiFi_secrets.h` and `backend_server_secrets_example.h` -> `backend_server_secrets.h` locally.
- Backend URL and access keys are referenced in `src_broker_esp32/src/tasks/httptransmission_task.cpp` and `backend_task.cpp`.

Project-specific conventions & patterns
- FreeRTOS multi-tasking: tasks, queues and event groups are central. See `src_broker_esp32/src/main.cpp` for global queue names: `dataQueue`, `networkQueue`, and `networkEventGroup`.
- Thread-safe logging uses `utils/threadsafe_serial.*` and a global `serialMutex` — prefer these helpers when adding prints.
- Circular buffer on the sensor side: `buffer_manager` implements enqueue/dequeue with global `queue_count` and APIs `addPacketToBuffer`/`getPacketFromBuffer`.
- BLE: UUIDs and characteristic names live in `src_package_arduino/include/bluetooth_manager.h`. When extending BLE behavior, follow existing advertise/subscribe patterns.

Integration points to pay attention to
- BLE → Broker: sensor publishes notifications; broker subscribes and writes SensorPacket structs into `dataQueue` (see `broker_task.cpp`). Keep struct layout compatible across projects (`sensor_data.h` / `sensor_packet_from_sensor.h`).
- JSON layout and HTTP: backendTask formats payloads — if you change fields, update both broker processing (`backend_task.cpp`) and server expectations.

Quality & tooling notes
- PlatformIO uses `clang-tidy` as `check_tool` with custom checks in `platformio.ini`. New C++ code should be clang-tidy clean where possible.
- Doxygen is used for docs. Run `doxygen Doxyfile` at repo root to regenerate documentation.

Quick examples to reference in code changes
- Add a new field to packets: update `sensor_data.h` (shared layout), then adapt `src_package_arduino/src/sensor_package_manager.cpp` (assembly) and `src_broker_esp32/src/tasks/backend_task.cpp` (JSON mapping).
- Change backend endpoint: update constant in `httptransmission_task.cpp` or provide `backend_server_secrets.h` locally.

Where to look first when assigned a task
1. `README.md` — project overview, hardware, and quick commands.
2. `src_broker_esp32/src/main.cpp` — shows runtime wiring of queues and tasks.
3. `src_package_arduino/include/*` — BLE UUIDs and packet assembly helpers.
4. `src_broker_esp32/src/tasks/*` — backend, broker, http transmission, network status: the core logic lives here.

If anything in this file is unclear or missing (e.g., additional env names, CI steps), tell me which area to expand and I will update this doc.
