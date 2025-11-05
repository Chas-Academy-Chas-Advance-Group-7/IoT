# Backend & JSON Format

This document explains the backend communication, JSON payload structure, and configuration for the **IoT Sensor System**.

---

## 1. Example JSON Payload

The ESP32 broker converts `SensorPacket`s to JSON before sending to the backend. Example payload:

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
```

> Derived from [`backend_task.cpp`](../src_broker_esp32/src/tasks/backend_task.cpp) in `src_broker_esp32/src/tasks/`.

---

## 2. Backend Communication

* **Backend task**: converts `SensorPacket` → JSON. (`backend_task.cpp`)
* **HTTP Transmission task**: sends JSON via HTTPS to the backend. (`httptransmission_task.cpp`)
* **Backend URL constant**: `BACKEND_URL` in `httptransmission_task.cpp` require `backend_server_secrets.h`
* **TLS & credentials**: loaded from `backend_server_secrets.h` and `WiFi_secrets.h`

### Notes

* Verify backend JSON schema matches your endpoint before modifying packet structure.
* Maintain consistent `SensorPacket` layout across Arduino and ESP32 to avoid misalignment.
* Do **not** hardcode secrets in source files; use local header files.

---

## 3. Queue & Reliability Considerations

* `dataQueue` stores incoming BLE packets.
* `networkQueue` stores JSON objects ready to transmit.
* Increase queue sizes in [`src_broker_esp32/src/main.cpp`](../src_broker_esp32/src/main.cpp) if experiencing packet drops.

---

## 4. Best Practices

* Test backend endpoint with `curl` before full integration.
* Confirm access keys and `truck_id` are correct.
* Ensure FreeRTOS tasks handle queue overflows gracefully to avoid crashes.
* Monitor logs via serial to track JSON generation and transmission.

This backend design ensures robust and consistent data transmission from multiple BLE sensors to a central cloud server.
