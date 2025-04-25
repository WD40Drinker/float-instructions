const int IN3 = 11; // Extend
const int IN4 = 10; // Retract
const int ENB = 9;  // Motor speed


int actuateFrom = 0;
int actuateTo = 0;


bool isMoving = false;
unsigned long movementStart = 0;
unsigned long movementDuration = 0;


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
  // Handle incoming Serial commands
  if (Serial.available()) {
    String inputString = Serial.readStringUntil('\n'); // Read full line
    inputString.trim(); // Remove whitespace


    if (inputString.length() > 0) {
      int input = inputString.toInt();


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


  // Handle timing and stop when done
  if (isMoving) {
    if (millis() - movementStart >= movementDuration) {
      stopMotor();
      actuateFrom = actuateTo; // ✅ ONLY update when move completes
      Serial.print("Reached ");
      Serial.print(actuateFrom);
      Serial.println("%");
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


}


void stopMotor() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0);
  isMoving = false;
}

