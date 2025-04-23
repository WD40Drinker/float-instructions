#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>

SoftwareSerial Bluetooth(3,2);

//when adjusting depth, make more boyant for incremement time and then make neutrally boyant

struct Data {
long time = -1;
float temp = -1;
float depth = -1;
float pressure = -1;
float altitude = -1;
};

// declare temporary sensor variables
String readDepth = "";
int toDepth;
long time;
float temp;
float depth;
float pressure;
float altitude;
Data collectedData[50]; // we will save our collected data into this array (we probably only need 500 points anyway)
int entry = 0; // location of the entry in the collectedData array

String incomingMessage = ""; // buffer for any data from mission station
MS5837 sensor;
// declare H-driver variables
const int IN3 = 11; // Extend
const int IN4 = 10; // Retract
const int ENB = 9;  // Motor speed
int actuateFrom = 0;
int actuateTo = 0;
bool isMoving = false;
unsigned long movementStart = 0;
unsigned long movementDuration = 0;
const int neutral = 50;

void setup() {
// put your setup code here, to run once:
  Serial.begin(38400);
  Serial.println("Starting");
  Wire.begin();
  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997);
  Bluetooth.begin(38400);

  //setup H-driver pins
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
  digitalWrite(IN4,HIGH);
  analogWrite(ENB,255);
  delay(8000);
  stopMotor();

  // mid our the H-driver
  forceMid(); // we need a function to make the H-driver neutral
}

void loop() {
  if(Bluetooth.available()){
    Serial.println("Bluetooth Connected:");
    Bluetooth.write("Team 2: Help, i am unda da water");
    time = millis();
    pressure = sensor.pressure();
    depth = sensor.depth();
    Bluetooth.write(time);
    Bluetooth.write("pressure: ");
    Bluetooth.write(pressure);
    Bluetooth.write("depth: ");
    Bluetooth.write(depth);
    Bluetooth.write("enter \"101\" to verify and start dive");

    bool verification = true; // while loop waits for user verification from mission station
    while(verification){
      //timeNow = millis();
      while (Bluetooth.available()) {
        char c = Bluetooth.read();
        incomingMessage += c;
        // If we see a newline or carriage return, process the message
        if (c == '\n') {
          incomingMessage.trim();  // Remove whitespace/newline
          Serial.println("Received: " + incomingMessage);
          if (incomingMessage == "101") {
            Serial.println("Bluetooth Signal Validated");
            Bluetooth.write("Bluetooth Signal Validated");
            //communicateData();
            verification = false;  // Exit loop
          }
          incomingMessage = "";  // Reset message buffer
        }
      }
    }

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
          readDepth = incomingMessage; //This exist for any bullshit typing rules arduino may have idk;
          toDepth = readDepth.toInt(); //convert input to integer;
          waitForInput = false;
          incomingMessage = "";  // Reset message buffer
        }
      }
    }

    Bluetooth.println("Starting Dive");
    //do stuff to sink the float
    Bluetooth.write("float is going to" + toDepth);
    Serial.println("Starting Dive (for real this time)");

    startMovement(70); // make the float sink by making it heavier

    while(toDepth > depth){ // IMPORTANT: CHECK THIS: WILL TYPES BE A PROBLEM?
      delay(5000); //give the sensor time to update its depth 
      saveData();
      //depth = sensor.depth();
    }
    //idk how to program adjustments when the float overshoots
    //This is the depth. Fuck you :)

    //delay(5000); // lets pause just because

    //make the float surface 
    Bluetooth.write("float is surfacing");
    startMovement(neutral - 20);

    while(depth > 50){
      delay(100);
      depth = sensor.depth();
    }

    //send the saved info
    Bluetooth.write("Dive completed successfully");

    startMovement(neutral + 20);
    dataDump();

    forceMid();
  }
}

void saveData() {
  temp = sensor.temperature();
  depth = sensor.depth();
  pressure = sensor.pressure();
  altitude = sensor.altitude();
  time = millis();
  String Stringtime = String(time);
  collectedData[entry].time = time;
  collectedData[entry].temp = temp;
  collectedData[entry].depth = depth;
  collectedData[entry].pressure = pressure;
  collectedData[entry].altitude = altitude; // lol you could totally clean this up :)
  //print to serial for testing purposes 
  Serial.println(collectedData[entry].time);
  Serial.println("Temp: ");
  Serial.println(collectedData[entry].temp);
  Serial.println("Depth: ");
  Serial.println(collectedData[entry].depth);
  Serial.println("Pressure: ");
  Serial.println(collectedData[entry].pressure);
  Serial.println("Altitiude: ");
  Serial.println(collectedData[entry].altitude);
  entry++;
}

void dataDump(){
  Bluetooth.write("time | temp | depth | pressure | altitiude");
  for(int i = 0; i < 50; i++){
    if(collectedData[i].time != -1){
      Bluetooth.write(collectedData[i].time);
      Bluetooth.write(collectedData[i].temp);
      Bluetooth.write(collectedData[i].depth);
      Bluetooth.write(collectedData[i].pressure);
      Bluetooth.write(collectedData[i].altitude);
      Bluetooth.write("\n");
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

void forceMid() {
  Serial.println("starting mid routine");
  startMovement(neutral);
}


