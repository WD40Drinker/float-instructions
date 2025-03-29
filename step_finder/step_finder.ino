#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>

//This program is desgned so we can measure the number of steps it takes to max out a H-driver given a certain increment

SoftwareSerial Bluetooth(3,2);

// declare H-driver variables
int enA = 11;
int in1 = 10;
int in2 = 9;

// declare other variables
int step = 0; // This will be our actuator position tracking variables.  step is our counter
int stepMax = 0;  // and stepMax will be our limit
int increment = 0; // this will be time our functions actuateForware() and actualteBackward() will run for

void setup() {
  // put your setup code here, to run once:

  serial.begin(9600);
  Serial.println("Starting");
  Wire.begin();
  //sensor.setModel(MS5837::MS5837_30BA);
  //sensor.setFluidDensity(997);
  // pretty sure we don't need this

  // Turn off motors - Initial state
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  // min our the H-driver
  startupMin();
  step = 0; // never hurts to be redundant
  
  Serial.println("Please enter increment time (ms)");
  increment = Serial.parseInt();

}

void loop() {
  // put your main code here, to run repeatedly:
  actuateForward();
  delay(increment);
  coastStop();
  Serial.println("Step"+ step + "\n"); // report to user (us)
  delay(3000); 
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

void starupMin() {
  bool continue = false;

  Bluetooth.write("Would you like to min out the H-driver Y/N \n");
  char input = (Bluetooth.read)[0];

  if(input == 'y' || input == 'Y'){
    continue = true;
    Bluetooth.wire("Starting H-driver min routine. \n");
  }

  while(continue){
    actuateBackward();
    delay(increment);
    coastStop();
    delay(1000);
        
    Bluetooth.write("Would you like to continue? Y/N \n");
    input = (Bluetooth.read)[0];

    if(input != 'y' && input != 'Y'){
      continue = false;
      Bluetooth.wire("Ending H-driver min routine \n");
    }
  }
}