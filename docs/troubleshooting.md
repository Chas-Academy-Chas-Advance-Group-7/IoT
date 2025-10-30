# Troubleshooting & Common Pitfalls

This document lists common issues, debugging steps, and best practices for the **IoT Sensor System**.

---

## 1. Serial Logging

* Multiple FreeRTOS tasks write to Serial on the ESP32 broker.
* Use thread-safe utilities in `utils/threadsafe_serial.*` and the global `serialMutex` (see `src_broker_esp32/src/utils/threadsafe_serial.cpp`).
* Ensures logs from different tasks do not interleave or corrupt.

---

## 2. BLE Communication Issues

* If the ESP32 broker isn’t receiving packets:

  * Verify that the BLE UUIDs in `bluetooth_manager.h` match on both the Arduino sensor and ESP32 hub.
  * Ensure the sensor is advertising and a BLE central connection is established.

---

## 3. Packet or JSON Mismatch

* If received JSON values appear wrong:

  * Check `SensorPacket` struct layout and endianness in `sensor_data.h`.
  * Verify packet assembly in `sensor_package_manager.h` on Arduino.
  * Make sure the ESP32 hub uses the correct structure for deserialization.

---

## 4. Data Drops & Queue Sizes

* Default queue depths (`dataQueue` and `networkQueue`) are small (10).
* If packets are dropped under high sensor frequency, increase queue depths in `src_broker_esp32/src/main.cpp`.

---

## 5. Network Failures

* `Networkstatus_task` automatically reconnects Wi-Fi if disconnected.
* If repeated failures occur, check credentials in `WiFi_secrets.h`.
* Verify router connectivity and signal strength.

---

## 6. Documentation Generation

* Generate API and developer docs with Doxygen:

```bash
doxygen Doxyfile
```

* Output HTML is typically stored in `/docs` or configured `OUTPUT_DIRECTORY`.

---

## 7. General Tips

* Always test backend endpoint with `curl` or Postman before full integration.
* Check serial logs for both sensor and hub when debugging BLE and packet issues.
* Confirm BLE connections are established before expecting data flow.
* For battery-powered setups, ensure voltage stability to prevent resets.


