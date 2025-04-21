#include "SevSeg.h"
#include <Wire.h>

SevSeg sevseg;
const int buzzer = A0;  // Define buzzer pin at A0
const int fanPin = A1;  // Define fan control pin at A1

//I2C Communication
#define SLAVE_ADDRESS 8
#define ANSWERSIZE 4

bool isSystemOn = false;
bool isCountdownComplete = false;  // New flag to track countdown completion

void setup() {
  Serial.begin(9600);
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);  // Add handler for master requests

  byte numDigits = 4;
  byte digitPins[] = {2, 3, 4, 5};
  byte segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};
  bool resistorsOnSegments = 0;

  sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins, resistorsOnSegments);
  sevseg.setBrightness(90);

  pinMode(buzzer, OUTPUT);
  pinMode(fanPin, OUTPUT);
  digitalWrite(fanPin, LOW);
}

void loop() {
  if (!isSystemOn) {
    // Reset completion flag when system is off
    isCountdownComplete = false;
    digitalWrite(fanPin, LOW);
    noTone(buzzer);
    sevseg.blank();
    return;
  }

  if (isSystemOn && !isCountdownComplete) {  // Only start countdown if not already complete
    // Countdown from 10
    for (int count = 10; count > 0; count--) {
      sevseg.setNumber(count, 0);
      tone(buzzer, 1000);
      delay(100);
      noTone(buzzer);

      unsigned long startTime = millis();
      while (millis() - startTime < 1000) {
        sevseg.refreshDisplay();
        if (!isSystemOn) {
          return;
        }
      }
    }

    // Final buzzer and display
    sevseg.setNumber(0, 0);
    tone(buzzer, 1000);
    unsigned long buzzStartTime = millis();
    while (millis() - buzzStartTime < 3000) {
      sevseg.refreshDisplay();
      if (!isSystemOn) {
        return;
      }
    }
    noTone(buzzer);

    // Turn on fan and set completion flag
    digitalWrite(fanPin, HIGH);
    isCountdownComplete = true;  // Mark countdown as complete

    // Display the date
    int date = 1123;
    sevseg.setNumber(date);

    delay(15000);
  }

  // Keep refreshing display after countdown
  sevseg.refreshDisplay();
}

// Handler for receiving commands from master
void receiveEvent(int bytes) {
  char received[ANSWERSIZE];
  int i = 0;

  while (Wire.available() && i < sizeof(received)) {
    received[i++] = Wire.read();
  }
  received[i] = '\0';

  if (strcmp(received, "ON") == 0) {
    isSystemOn = true;
    isCountdownComplete = false;  // Reset completion flag when turning on
    Serial.println("System is now ON.");
  } else if (strcmp(received, "OFF") == 0) {
    isSystemOn = false;
    Serial.println("System is now OFF.");
    digitalWrite(fanPin, LOW);
    noTone(buzzer);
    sevseg.blank();
  }
  Serial.println(received);
}

// Handler for responding to master requests
void requestEvent() {
  if (isCountdownComplete) {
    Wire.write("Done");  // Send "Done" when countdown is complete
  } else {
    Wire.write("Run ");  // Send "Run " (with space to match 4 chars) when still running
  }
}