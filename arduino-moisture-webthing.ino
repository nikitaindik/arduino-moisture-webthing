#include <Arduino.h>
#include "Thing.h"
#include "WebThingAdapter.h"

char *SSID = "";
char *PASSWORD = "";

int MIN_READING = 340; // Dry
int MAX_READING = 740; // Water

const int LED_PIN = 13;

WebThingAdapter *adapter;

const char *mySoilMoistureSensorTypes[] = {"MultiLevelSensor", nullptr};
// ThingDevice - id, title, type
ThingDevice mySoilMoistureSensor("mySoilMoistureSensor", "My Soil Moisture Sensor", mySoilMoistureSensorTypes);
// ThingProperty - id, description, thingDataType, atType, callback
ThingProperty moistureLevelProperty("moistureLevel", "Moisture level", NUMBER, "LevelProperty");

void connectToWifi(char *SSID, char *PASSWORD)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);

    Serial.println("");
    Serial.print("Connecting to \"");
    Serial.print(SSID);
    Serial.println("\"");
    Serial.println("");

    bool blink = true;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        digitalWrite(LED_PIN, blink ? LOW : HIGH); // active low led
        blink = !blink;
    }
    digitalWrite(LED_PIN, HIGH); // active low led

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(SSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup(void)
{
    pinMode(A0, INPUT);

    Serial.begin(115200);

    connectToWifi(SSID, PASSWORD);

    moistureLevelProperty.minimum = 0;
    moistureLevelProperty.maximum = 100;
    moistureLevelProperty.unit = "percent";
    mySoilMoistureSensor.addProperty(&moistureLevelProperty);

    adapter = new WebThingAdapter("w25", WiFi.localIP());
    adapter->addDevice(&mySoilMoistureSensor);
    adapter->begin();

    Serial.println("HTTP server started");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.print("/things/");
    Serial.println(mySoilMoistureSensor.id);
}

void loop(void)
{
    int rawReading = analogRead(A0);
    int clampedReading;

    if (rawReading < MIN_READING)
    {
        clampedReading = MIN_READING;
    }
    else if (rawReading > MAX_READING)
    {
        clampedReading = MAX_READING;
    }
    else
    {
        clampedReading = rawReading;
    }

    int percentage = map(clampedReading, MIN_READING, MAX_READING, 100, -1);

    Serial.print("Reading: ");
    Serial.print(clampedReading);
    Serial.print(" Value: ");
    Serial.print(percentage);
    Serial.print("\n");

    ThingPropertyValue levelValue;
    levelValue.number = percentage;
    moistureLevelProperty.setValue(levelValue);

    adapter->update();

    delay(1000);
}