#include "DHT_handler.h"

const uint8_t DHT_PIN = 2;
const uint8_t DHT_TYPE = DHT11;

float temperature = 0.0;
float humidity = 0.0;

DHT dht(DHT_PIN, DHT_TYPE);

void initializeDHT()
{
    dht.begin();
    Serial.println("DHT initialized!");
}