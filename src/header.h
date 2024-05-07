#include "ESPAsyncWebServer.h"
#include "WebSocketsServer.h"
#include "ArduinoJson.h"
#include "ESPmDNS.h"
#include "HTTPClient.h"

#include "htmlPage.h"
#include "secret.h"

AsyncWebServer server(80);
WebSocketsServer webSocket(81);

const char *ssid = SSID;
const char *pass = PASS;

const String thingSpeakUrl = THINGSPEAKURL1;
const String thingSpeakUrl2 = THINGSPEAKURL2;

int humidity, temperature, air;
float rzero;
String airStatus;

JsonDocument json;
String jsonString;

void flush() {
    json.clear();
    jsonString = "";
}

void wifiSetup() {
    WiFi.begin(ssid, pass);
    Serial.print("\nConnecting to wifi ");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(500);
    }
    Serial.println("\nConnected to wifi " + (String) ssid);
    Serial.println("IP Address: " + WiFi.localIP().toString());
}

void mdnsSetup() {
    Serial.println(
            !MDNS.begin("esp")
            ? "Error setting up mDNS responder!"
            : "mDNS responder started"
    );
}

void serverSetup() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", htmlPage);
    });
    server.on("/api/monitoring", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", jsonString);
    });
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    });
    server.begin();
}

void websocketSetup() {
    webSocket.begin();
    webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
        switch (type) {
            case WStype_DISCONNECTED:
                Serial.printf("[%u] Disconnected!\n", num);
                break;
            case WStype_CONNECTED: {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
                webSocket.sendTXT(num, "Connected");
            }
                break;
            case WStype_TEXT:
                Serial.printf("[%u] Received text: %s\n", num, payload);
                webSocket.sendTXT(num, payload);
                break;
            default:
                break;
        }
    });
}

void requestUpdate(String url);

unsigned long previousMillis_thing;
unsigned long currentMillis_thing;

void thingSpeakUpdate() {
    currentMillis_thing = millis();
    if (WiFi.status() == WL_CONNECTED && currentMillis_thing - previousMillis_thing >= 2000) {
        previousMillis_thing = currentMillis_thing;
        if (WiFi.status() == WL_CONNECTED) {
            requestUpdate(thingSpeakUrl);
            requestUpdate(thingSpeakUrl2);
        } else {
            Serial.println("Wifi disconnected");
        }
    }
}

void requestUpdate(String url) {
    HTTPClient http;
    String request = url + "&field1=" + String(temperature) + "&field2=" + String(humidity) + "&field3=" + String(air) + "&field4=" + String(rzero);
    http.begin(request);
    int httpResponseCode = http.GET();

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.println(httpResponseCode == 200 ? jsonString : "BAD_REQUEST");

    http.end();
}