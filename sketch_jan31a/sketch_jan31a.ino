#include <Wire.h>
#include "MS5837.h"
#include <SoftwareSerial.h>

SoftwareSerial Bluetooth(3,2);

// declare sensor variables
int toDepth;
int temp;
int depth;
int pressure;
int altitude;
MS5837 sensor;

// declare H-driver variables
int enA = 11;
int in1 = 10;
int in2 = 9;

// declare other variables
int step = 0; // This will be our actuator position tracking variables.  step is our counter
int stepMax = 10;  // and stepMax will be our limit
int increment = 5000; // this will be time our functions actuateForware() and actualteBackward() will run for

String message = "Dive completed successfully";

void setup() {
// put your setup code here, to run once:

  serial.begin(9600);
  Serial.println("Starting");
  Wire.begin();
  sensor.setModel(MS5837::MS5837_30BA);
  sensor.setFluidDensity(997);

  Serial.println("HC-05 AT Commands:");
  Serial.println("Function            	| Command       	| Response     	   | Parameter");
  Serial.println("Test                	| AT            	| OK           	   | None");
  Serial.println("Check Master/Slave  	| AT+CMODE?     	| +CMOD:<cmod> 	   | 0 (Default)");
  Serial.println("Set Master/Slave mode | AT+CMODE=<cmode>      | OK           	   | 0 (slave) OR 1 (master))");
  Serial.println("Get Address         	| AT+ADDR?      	| +ADDR:<addr>, OK | None");
  Serial.println("Connect to Address  	| AT+BIND=<addr>	| OK           	   | Replace ':' with ','");
  Serial.println("Reset Device        	| AT+RESET      	| OK           	   | None");
  Bluetooth.begin(38400); //HC-05 default speed in AT command mode

  Bluetooth.begin(38400); //HC-05 default speed in AT command mode

  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  // Turn off motors - Initial state
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  // min our the H-driver
  startupMin();
  step = 0; // never hurts to be redundant

}

void loop() {
  if(Bluetooth.available()){
	  message = "Dive completed successfully";
    //do stuff to sink the float
    toDepth = stoi(Bluetooth.read); //I don't remember do I have to typecast this? 
    serial.println("float is going to" + toDepth);

    while(toDepth > depth){
      //increment down with the actuator
      delay(2500); // delay to upload new instructions  idk if I really need this 
      actuateForward();
      delay(increment);
      coastStop();
      delay(1000); //give the sensor time to update its depth 
      depth = sensor.depth();
	    if(step >= stepMax){
		    message = "Cannot go to depth: " + toDepth;
		    break;
	    }
    }
    //idk how to program adjustments when the float overshoots
    //This is the depth. Fuck you :)

    delay(5000); // lets pause just because
    //save info from the float
    temp == sensor.temperature();
    depth == sensor.depth();
    pressure == sensor.pressure();
    altitude == sensor.altitude();
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
    Bluetooth.write(message + "\n"); // send info back to the other arduino 
    Bluetooth.write("The Temperature is " + temp + " deg C \n" + 
    "The Depth is " + depth + " m \n" + 
    "The Pressure is " + pressure + 
    " mbar \n" + "The Altitiude is" 
    + altitude + "m above mean sea level ");
  }

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



