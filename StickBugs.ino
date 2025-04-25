#include <ArduinoBLE.h>
#include "DFRobot_DHT11.h"

#define DHT11_PIN  4        // Pin where the data line is connected

const byte nsum = 10;
int atomizationPin = 2;

unsigned long previousMillis = 0;
const unsigned long readInterval = 5000;  // Read every 5 seconds
const unsigned long humidifierRunTime = 10000;
bool humidifierRunning = false;
unsigned long humidifierStartTime = 0;
float minimalHumidity = 60.0;

BLEService envService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEFloatCharacteristic humidityChar("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEFloatCharacteristic temperatureChar("19B10002-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);

void setup() {
  Serial.begin(9600);
  pinMode(atomizationPin, OUTPUT);
  digitalWrite(atomizationPin, LOW);

  delay(1000);
  Serial.println("StickBugs Environment Monitor");

  if (!BLE.begin()) {
    Serial.println("BLE failed to start.");
    while (1);
  }

  BLE.setLocalName("StickBugsBLE");
  BLE.setAdvertisedService(envService);
  envService.addCharacteristic(humidityChar);
  envService.addCharacteristic(temperatureChar);  // Add temperature characteristic
  BLE.addService(envService);
  BLE.advertise();
  Serial.println("BLE started and advertising.");
}

void loop() {
  BLE.poll();
  unsigned long currentMillis = millis();

  if (humidifierRunning && (currentMillis - humidifierStartTime >= humidifierRunTime)) {
    digitalWrite(atomizationPin, LOW);
    humidifierRunning = false;
    Serial.println("Humidifier turned OFF");
  }

  if (!humidifierRunning && (currentMillis - previousMillis >= readInterval)) {
    previousMillis = currentMillis;

    DFRobot_DHT11 dht;
    dht.read(DHT11_PIN); // Read values from the sensor

    float humidity = dht.humidity;
    float temperature = dht.temperature;

    Serial.println("-------- READINGS --------");
    Serial.print("Humidity: ");
    Serial.print(humidity, 0);
    Serial.println(" %RH");
    Serial.print("Temperature: ");
    Serial.print(temperature, 0);
    Serial.println(" °C");
    
    humidityChar.writeValue(humidity);
    temperatureChar.writeValue(temperature);  // Write temperature value to BLE

    if (humidity < minimalHumidity) {
      //digitalWrite(atomizationPin, HIGH);
      humidifierRunning = true;
      humidifierStartTime = currentMillis;
      Serial.println("Humidity low! Humidifier turned ON");
    }
  }
}