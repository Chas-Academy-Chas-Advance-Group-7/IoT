# IoT Sensor System

## Overview

This project implements a complete IoT sensor system with **two units**:

1. **Sensor Unit:** Arduino Uno WiFi Rev 4 equipped with a DHT11 sensor, broadcasting temperature and humidity data via **BLE**.  
2. **Central Hub:** ESP32 device that scans for BLE sensors, processes data, and sends it to a backend server via **Wi-Fi HTTP**.  

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

## Project Structure

## Documentation

Documentation is generated with **Doxygen**. To generate the docs, run:

```bash
doxygen Doxyfile
```

