#include "header.h"

#define led_buzzer 2
#define MQ135PIN 32
#define DHTPIN 4

MQ135 mq135_sensor(MQ135PIN);
DHTesp dht;

void setup() {
    Serial.begin(115200);
    setCpuFrequencyMhz(240);
    lcd.begin(16, 2);
    delay(1000);
    dht.setup(DHTPIN, DHTesp::DHT11);
    delay(1000);
    pinMode(led_buzzer, OUTPUT);
    wifiSetup();
    serverSetup();
    websocketSetup();
    mdnsSetup();
    interruptSetup();
}

unsigned long previousMillis;
unsigned long currentMillis;

unsigned long previousMillisforSocket;
unsigned long currentMillisforSocket;

void loop() {

    currentMillisforSocket = millis();
    if (currentMillisforSocket - previousMillisforSocket >= 25) {
        previousMillisforSocket = currentMillisforSocket;
        webSocket.loop();
    }

    currentMillis = millis();
    if (currentMillis - previousMillis >= 1000) {
        previousMillis = currentMillis;

        humidity = dht.getHumidity();
        temperature = dht.getTemperature();
        if (isnan(humidity) || isnan(temperature)) {
            Serial.println("Failed to read from DHT sensor!");
            return;
        }
        air = mq135_sensor.getPPM();
        rzero = mq135_sensor.getRZero();
        Serial.println("-------rzero-------");
        Serial.print("rzero: ");
        Serial.println(rzero);
        Serial.println("--------------");
        air = air > 5200 ? 5200 : air;
        airStatus = (air <= 1000) ? "Good Air" : ((air > 1000 && air <= 2000) ? "Bad Air" : "Danger!");
        digitalWrite(led_buzzer, air > 2000);
        flush();
        json["humidity"] = (String)humidity + " %";
        json["temperature"] = (String) temperature + " °C";
        json["air"] = (String) air + " ppm";
        json["status"] = airStatus;
        serializeJson(json, jsonString);
        webSocket.broadcastTXT(jsonString);
    }
    thingSpeakUpdate();
    printSensorDataOnLcd();
}