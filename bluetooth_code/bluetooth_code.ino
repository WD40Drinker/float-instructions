#include <SoftwareSerial.h>


SoftwareSerial Bluetooth(3,2); // RX, TX

void setup() {
  Serial.begin(38400);


  Serial.println("HC-05 AT Commands:");
  Serial.println("Function                | Command           | Response         | Parameter");
  Serial.println("Test                    | AT                | OK               | None");
  Serial.println("Check Master/Slave      | AT+CMODE?         | +CMOD:<cmod>     | 0 (Default)");
  Serial.println("Set Master/Slave mode   | AT+CMODE=<cmode>  | OK               | 0 (slave) OR 1 (master)");
  Serial.println("Get Address             | AT+ADDR?          | +ADDR:<addr>, OK | None");
  Serial.println("Connect to Address      | AT+BIND=<addr>    | OK               | Replace ':' with ','");
  Serial.println("Get Binded Address      | AT+BIND?          | +BIND:<addr>, OK | None");
  Serial.println("Reset Device            | AT+RESET          | OK               | None");
  Serial.println("\nEnter AT Commands:");
 
  Bluetooth.begin(38400); //HC-05 default speed in AT command mode
}


void loop() {
  //read from HC-05 and send data to the Arduino Serial Monitor
  if (Bluetooth.available()) {
    Serial.write(Bluetooth.read());
  }

  //Read from Arduino Serial Monitor and send to HC-05
  if (Serial.available()) {
    Bluetooth.write(Serial.read());
  }
}
