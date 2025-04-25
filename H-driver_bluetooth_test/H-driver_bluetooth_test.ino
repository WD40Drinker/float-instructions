#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>

const int IN3 = 11; // Extend
const int IN4 = 10; // Retract
const int ENB = 9;  // Motor speed

int actuateFrom = 0;
int actuateTo = 0;

bool isMoving = false;
unsigned long movementStart = 0;
unsigned long movementDuration = 0;

SoftwareSerial Bluetooth(3,2);
String incomingMessage = "";
String readPercent = "";
int toPercent = 0;

void setup() {
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
  digitalWrite(IN4,HIGH);
  analogWrite(ENB,255);
  delay(8000);
  Serial.begin(9600);
  stopMotor();

  Serial.println("Ready. Enter a target percent (0-100):");
}

void loop() {
  bool waitForInput = true; // while loop waits for input from mission station
  while(waitForInput){
    while (Bluetooth.available()) {
      char c = Bluetooth.read();
      if(isDigit(c)){
        incomingMessage += c;
      }
      // If we see a newline or carriage return, process the message
      if (c == '\n') {
        incomingMessage.trim();  // Remove whitespace/newline
        Serial.println("Received: " + incomingMessage);
        readPercent = incomingMessage; //This exist for any bullshit typing rules arduino may have idk;
        //toPercent = readPercent.toInt(); //convert input to integer;
        waitForInput = false;
        incomingMessage = "";  // Reset message buffer
      }
    }
  }

    if (readPercent.length() > 0) {
      int input = readPercent.toInt();

      if (input >= 0 && input <= 100) {
        if (input != actuateFrom) {
          startMovement(input);
        } else {
          Serial.print("Already at ");
          Serial.print(actuateFrom);
          Serial.println("%");
        }
      } else {
        Serial.println("Invalid input. Please enter a value from 0 to 100.");
      }
    }
}

void startMovement(int i) {
	actuateTo = i;
  int diff = actuateTo - actuateFrom;

  Serial.print("Moving from ");
  Serial.print(actuateFrom);
  Serial.print("% to ");
  Serial.print(actuateTo);
  Serial.println("%");

  movementStart = millis();
  movementDuration = abs(diff) * 70;  // Tune this: ms per % change

  if (diff > 0) {
    // Extend
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else {
    // Retract
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }

  analogWrite(ENB, 200); // PWM speed
  isMoving = true;

	// put while loop here 
	// when the stop happens/ escape while loop
    while (isMoving) {
    if (millis() - movementStart >= movementDuration) {
      stopMotor();
      actuateFrom = actuateTo; // ✅ ONLY update when move completes
      Serial.print("Reached ");
      Serial.print(actuateFrom);
      Serial.println("%");
    }
  }
}

void stopMotor() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0);
  isMoving = false;
}

