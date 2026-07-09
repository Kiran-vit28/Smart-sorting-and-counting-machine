#include "arduino_secrets.h"
/*
  IR Sensor Counter with Arduino IoT Cloud
  Sends live object count to the Cloud variable "count"
*/

#include "thingProperties.h"

const int irSensorPin = 2;   // Use GPIO 14 if ESP32
int lastIrState = HIGH;
int countLocal = 0;

void setup() {
  Serial.begin(9600);
  delay(1500);

  // Initialize Cloud properties
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  pinMode(irSensorPin, INPUT);
  Serial.println("IR Sensor Counter Started...");
}

void loop() {
  ArduinoCloud.update();

  int irState = digitalRead(irSensorPin);

  // Detect object (falling edge trigger)
  if (lastIrState == HIGH && irState == LOW) {
    countLocal++;
    count = countLocal;   // Update Cloud variable
    Serial.print("Object Detected! Total Count: ");
    Serial.println(countLocal);
  }

  lastIrState = irState;
  delay(100); // debounce delay
}
