#include "DHT_handler.h"
#include <Arduino.h>

void setup()
{
    Serial.begin(9600);
    initializeDHT();
}

void loop()
{
    Serial.println("Hello, Arduino!");

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    delay(1000);
}
