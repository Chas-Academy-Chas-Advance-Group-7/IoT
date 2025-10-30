# System Architecture

This document explains the high-level design and component structure of the **IoT Sensor System**, including both the Sensor Unit (Arduino Uno WiFi Rev 4) and the Central Hub (ESP32).

The architecture ensures reliable, real-time data acquisition, BLE communication, and secure HTTP transmission to a backend.

---

## 1. Sensor Unit (Arduino Uno WiFi Rev 4)

### Modules & Responsibilities

* **DHT_handler**: Reads temperature and humidity from the DHT11 sensor.
* **Sensor_package_manager**: Creates `SensorPacket` objects containing sensor data.
* **Bluetooth_manager**: Manages BLE service and characteristics; advertises sensor data and sends notifications.
* **Buffer_manager**: Implements a circular buffer to queue unsent packets and prevent data loss.
* **Sensor_state_manager**: Implements a state machine that controls the sensor’s reading, packet creation, buffering, and BLE transmission.
* **Main Loop**: Continuously runs the state machine and handles BLE transmission logic.

### Data Flow

1. Read temperature and humidity via `DHT_handler`.
2. Assemble a `SensorPacket` with `Sensor_package_manager`.
3. Place packet into `Buffer_manager` circular buffer.
4. Send packet via BLE using `Bluetooth_manager` when connected.

---

## 2. Central Hub (ESP32)

### Modules & Responsibilities

* **Backend_task**: Processes incoming sensor packets and formats them into JSON for the backend.
* **Broker_task**: Handles BLE scanning, connecting to sensors, and subscribing to notifications.
* **Networkstatus_task**: Monitors Wi-Fi connectivity and triggers automatic reconnection if necessary.
* **Httptransmission_task**: Sends JSON data to the backend server via HTTPS.
* **Threadsafe Serial Utilities**: Ensures safe logging from multiple FreeRTOS tasks to prevent serial conflicts.

### Data Flow

1. BLE scan for sensor advertisements via `Broker_task`.
2. Subscribe to sensor notifications and enqueue packets into `dataQueue`.
3. `Backend_task` converts `SensorPacket` to JSON and pushes it to `networkQueue`.
4. `Httptransmission_task` reads from `networkQueue` and sends HTTPS requests to the backend.
5. `Networkstatus_task` monitors Wi-Fi connection; triggers reconnects if disconnected.

---

## 3. Communication & Reliability Features

* **BLE Notifications**: Used for sensor-to-hub communication, reducing the need for constant polling.
* **Circular Buffers**: Prevent data loss when multiple packets arrive faster than they can be transmitted.
* **State Machine Logic**: Ensures sensors operate in a controlled and recoverable manner.
* **Thread-safe Logging**: Multiple tasks can log simultaneously without interfering with each other.
* **Backend JSON Conversion**: Ensures consistent payload format for HTTPS transmission.
* **Automatic Reconnection**: Wi-Fi and BLE reconnection handled automatically to maximize uptime.

---

## Notes & Best Practices

* Ensure consistent `SensorPacket` structure between Sensor Unit and Central Hub. Any changes require updates in both `sensor_package_manager` and `sensor_packet_from_sensor.h`.
* Verify BLE UUIDs (`SENSOR_SERVICE_UUID` and `SENSOR_CHAR_UUID`) match between the Arduino and ESP32.
* Adjust queue sizes (`dataQueue` and `networkQueue`) in ESP32 if packet drop is observed under high sensor frequency.
* Use serial monitor and thread-safe utilities to debug runtime behavior on ESP32.
* Doxygen comments in source files can be used to generate developer reference documentation.

---

This architecture provides a robust, modular system separating sensor acquisition, BLE communication, and backend transmission, while enabling maintainability and scalability.
