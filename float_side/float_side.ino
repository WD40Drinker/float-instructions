#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>

SoftwareSerial Bluetooth(3,2);

// declare sensor variables
String readDepth = "";
long time;
int toDepth;
float temp;
float depth;
float pressure;
float altitude;
String incomingMessage = "";
MS5837 sensor;
// declare H-driver variables
int enA = 11;
int in1 = 10;
int in2 = 9;

// declare other variables
int step = 0; // This will be our actuator position tracking variables.  step is our counter
int stepMax = 10;  // and stepMax will be our limit
int increment = 5000; // this will be time our functions actuateForware() and actualteBackward() will run for

String message = String("Dive completed successfully");

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
    Serial.println("Bluetooth Connected");
    Bluetooth.write("can you hear me?");
    Bluetooth.write("enter \"101\" to verify and start dive");

    bool verification = true;
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
          Bluetooth.println("000");
          communicateData();
          verification = false;  // Exit loop
        }
        incomingMessage = "";  // Reset message buffer
        }
      }
    }

    bool waitForInput = true;
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
	  message = "Dive completed successfully";
    //do stuff to sink the float
    readDepth = String(Bluetooth.read()); //I don't remember do I have to typecast this? 
    toDepth = readDepth.toInt();
    Bluetooth.write("float is going to" + toDepth);
    Serial.println("Starting Dive");

    while(toDepth > depth){
      //increment down with the actuator
      delay(1000); // delay to upload new instructions  idk if I really need this 
      actuateForward();
      delay(increment);
      coastStop();
      delay(1000); //give the sensor time to update its depth 
      communicateData();
      depth = sensor.depth();
	    if(step >= stepMax){
		    message = "Cannot go to depth: " + toDepth;
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
      delay(1000);
    }
    step = 0; // never hurts to be redundant

    //send the saved info
    Bluetooth.write(&message); // send info back to the other arduino 
    communicateData();
  }
}

void communicateData() {
  temp = sensor.temperature();
  depth = sensor.depth();
  pressure = sensor.pressure();
  altitude = sensor.altitude();
  time = millis();
  String Stringtime = String(time);
  Bluetooth.write(time);
  Bluetooth.write(temp);
  Bluetooth.write(depth);
  Bluetooth.write(pressure);
  Bluetooth.write(altitude);


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
  for(int i = 0; i < stepMax; i++){

    actuateBackward();
    delay(increment);
    coastStop();
  }
}



