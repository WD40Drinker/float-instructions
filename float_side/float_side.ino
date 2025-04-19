#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>

SoftwareSerial Bluetooth(3,2);


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
Data collectedData[500]; // we will save our collected data into this array (we probably only need 500 points anyway)
int entry = 0; // location of the entry in the collectedData array

String incomingMessage = ""; // buffer for any data from mission station
MS5837 sensor;
// declare H-driver variables
int enA = 11;
int in1 = 10;
int in2 = 9;

// declare other variables
int step = 0; // This will be our actuator position tracking variables.  step is our counter
int stepMax = 10;  // and stepMax will be our limit (if it exceeds it, the float will return)
int increment = 5000; // this will be time our functions actuateForware() and actualteBackward() will run for

bool reached = true; // determines the message printed after a dive is completed

void setup() {
// put your setup code here, to run once:
  Serial.begin(38400);
  Serial.println("Starting");
  Wire.begin();
  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997);

  Bluetooth.begin(38400); //HC-05 default speed in AT command mode

  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  // Turn off motors - Initial state
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  // min our the H-driver
  forceMin();
  step = 0; // never hurts to be redundant
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
      //timeNow = millis();
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

    Bluetooth.println("Starting Dive: ");
    //do stuff to sink the float
    readDepth = String(Bluetooth.read()); //I don't remember do I have to typecast this? 
    toDepth = readDepth.toInt();
    Bluetooth.write("float is going to" + toDepth);
    Serial.println("Starting Dive");

    while(toDepth > depth){ // IMPORTANT: CHECK THIS: WILL TYPES BE A PROBLEM?
      //increment down with the actuator
      delay(1000); // delay to upload new instructions  idk if I really need this 
      actuateForward();
      delay(increment);
      coastStop();
      delay(1000); //give the sensor time to update its depth 
      communicateData();
      depth = sensor.depth();
	    if(step >= stepMax){
		    reached = false;
		    break;
	    }
    }
    //idk how to program adjustments when the float overshoots
    //This is the depth. Fuck you :)

    delay(5000); // lets pause just because
    //empty the valve (min out the H-driver)
    Bluetooth.write("float is surfacing");
    for(int i = 0; i < step; i++){
      actuateBackward();
      delay(increment);
      coastStop();
      communicateData();
      delay(1000);
    }
    step = 0; // never hurts to be redundant

    //send the saved info
    if(reached== true){
      Bluetooth.write("Dive completed successfully");
    }
    else{
      Bluetooth.write("Could not reach depth " + toDepth);
    }

    communicateData();
    reached = true; // dive is done, reset reached variable
  }
}

void communicateData() {
  temp = sensor.temperature();
  depth = sensor.depth();
  pressure = sensor.pressure();
  altitude = sensor.altitude();
  time = millis();
  String Stringtime = String(time);
  //Bluetooth.write("Time: ");
  Bluetooth.write(time);
  Bluetooth.write("Temp: ");
  Bluetooth.write(temp);
  Bluetooth.write("Depth: ");
  Bluetooth.write(depth);
  Bluetooth.write("Pressure: ");
  Bluetooth.write(pressure);
  Bluetooth.write("Altitiude: ");
  Bluetooth.write(altitude);
  collectedData[entry].time = time;
  collectedData[entry].temp = temp;
  collectedData[entry].depth = depth;
  collectedData[entry].pressure = pressure;
  collectedData[entry].altitude = altitude; // lol you could totally clean this up :)
  entry++;
}

// declare H-driver functions
  void actuateForward() {
  analogWrite(enA, 255);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  step ++;
}

void actuateBackward() {
  analogWrite(enA, 255);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  step --;
}

void coastStop() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
}

void brakeStop() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, HIGH);
}

void forceMin() {
  Serial.println("starting min routine");
  for(int i = 0; i < stepMax; i++){
    actuateBackward();
    delay(increment);
    coastStop();
  }
}


