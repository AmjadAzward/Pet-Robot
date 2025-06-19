#include <Wire.h>

// Motor Pins
const int ENA = 9;
const int ENB = 10;
const int LeftMotorForward = 2;
const int LeftMotorBackward = 3;
const int RightMotorForwar_d = 5;
const int RightMotorBackward = 4;
int speedValue = 150;

// Bluetooth Mode Control
enum Mode { MANUAL, OBSTACLE, FOLLOW };
Mode currentMode = MANUAL;

void setup() {
  Serial.begin(9600);
  Serial.println("Master Arduino Ready");

  Wire.begin(); 

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(LeftMotorForward, OUTPUT);
  pinMode(LeftMotorBackward, OUTPUT);
  pinMode(RightMotorForward, OUTPUT);
  pinMode(RightMotorBackward, OUTPUT);

  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);
}

void moveForward() {
  Serial.println("Moving Forward");
  digitalWrite(LeftMotorForward, HIGH);
  digitalWrite(RightMotorForward, HIGH);
  digitalWrite(LeftMotorBackward, LOW);
  digitalWrite(RightMotorBackward, LOW);
}

void moveBackward() {
  Serial.println("Moving Backward");
  digitalWrite(LeftMotorBackward, HIGH);
  digitalWrite(RightMotorBackward, HIGH);
  digitalWrite(LeftMotorForward, LOW);
  digitalWrite(RightMotorForward, LOW);
}

void turnLeft() {
  Serial.println("Turning Left");
  digitalWrite(LeftMotorForward, LOW);
  digitalWrite(LeftMotorBackward, HIGH);
  digitalWrite(RightMotorForward, HIGH);
  digitalWrite(RightMotorBackward, LOW);
}

void turnRight() {
  Serial.println("Turning Right");
  digitalWrite(LeftMotorForward, HIGH);
  digitalWrite(LeftMotorBackward, LOW);
  digitalWrite(RightMotorForward, LOW);
  digitalWrite(RightMotorBackward, HIGH);
}

void stopMotors() {
  Serial.println("Stopping Motors");
  digitalWrite(LeftMotorForward, LOW);
  digitalWrite(LeftMotorBackward, LOW);
  digitalWrite(RightMotorForward, LOW);
  digitalWrite(RightMotorBackward, LOW);
}

void checkBluetoothCommands() {
  if (Serial.available()) {
    char command = Serial.read();
    Serial.print("Received: ");
    Serial.println(command);
    
    if (command == '1') currentMode = MANUAL;
    else if (command == '2') currentMode = OBSTACLE;
    else if (command == '3') currentMode = FOLLOW;

    if (currentMode == MANUAL) {
      switch (command) {
        case 'U': moveForward(); break;
        case 'D': moveBackward(); break;
        case 'L': turnLeft(); break;
        case 'R': turnRight(); break;
        case 'S': stopMotors(); break;
        case 'H':  // Send 'H' to Slave to turn on Buzzer
          Wire.beginTransmission(8);
          Wire.write('H');
          Wire.endTransmission();
          break;
        case 'P':  // Send 'P' to Slave to turn off Buzzer
          Wire.beginTransmission(8);
          Wire.write('P');
          Wire.endTransmission();
          break;
      }
    } 
  }

  if (currentMode == OBSTACLE) {
    Wire.beginTransmission(8);
    Wire.write('2');
    Wire.endTransmission();
    
    delay(100);

    Wire.requestFrom(8, 1);
    if (Wire.available()) {
      int distance = Wire.read();
      Serial.print("Obstacle Distance: ");
      Serial.println(distance);

      if (distance < 15) {
        stopMotors();
        delay(500);
        turnRight();
        delay(500);
      } else {
        moveForward();
      }
    }
  }
  if (currentMode == FOLLOW) {
    Wire.beginTransmission(8);  // Send request to Slave
    Wire.write('3');  // Request FOLLOW mode
    Wire.endTransmission();
    
    delay(100); // Wait for response
    
    Wire.requestFrom(8, 1);  // Expecting one byte
    if (Wire.available()) {
      char followCommand = Wire.read();
      Serial.print("Follow Command: ");
      Serial.println(followCommand);
      
      switch (followCommand) {
        case 'U': moveForward(); break;
        case 'L': turnLeft(); break;
        case 'R': turnRight(); break;
        case 'S': stopMotors(); break;
      }
    }
  }
}

void loop() {
  checkBluetoothCommands();
}