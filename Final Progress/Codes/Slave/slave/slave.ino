#include <Wire.h>
#include <Servo.h>

// Sensor Pins
const int TRIG_PIN = A1;
const int ECHO_PIN = A2;
const int LEFT_IR = 8;
const int RIGHT_IR = 7;
const int TOUCH_SENSOR = 11;
const int MQ_SENSOR = A0;
const int LED_PIN = 13;
const int MH_SENSOR = A3; 
const int BUZZER = 12;
const int HAND_SENSOR = A4;

// Servo Motors
Servo headServo;
Servo tailServo;

bool followModeActive = false;
bool obstacleModeActive = false; 

void setup() {
  Serial.begin(9600);
  Serial.println("Slave Arduino Ready");

  Wire.begin(8);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(sendResponse);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LEFT_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);
  pinMode(TOUCH_SENSOR, INPUT);
  pinMode(MQ_SENSOR, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(HAND_SENSOR, INPUT);

  headServo.attach(9);
  tailServo.attach(6);

  tailServo.write(90);
}

int getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;
  
  return (distance > 400) ? 400 : distance;
}

void checkLight() {
  if (analogRead(MH_SENSOR) > 800) {
    Serial.println("Dark environment detected, turning on LED.");
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

void receiveEvent(int howMany) {
  if (Wire.available()) {
    char mode = Wire.read();
    
    if (mode == '3') {
      Serial.println("Follow Mode Activated");
      followModeActive = true;
      obstacleModeActive = false;
    } 
    else if (mode == '2') {
      Serial.println("Obstacle Mode Activated");
      obstacleModeActive = true;
      followModeActive = false;
    } 
    else if (mode == 'H') {  // Turn on Buzzer
      Serial.println("Buzzer ON");
      digitalWrite(BUZZER, HIGH);
    } 
    else if (mode == 'P') {  // Turn off Buzzer
      Serial.println("Buzzer OFF");
      digitalWrite(BUZZER, LOW);
    } 
    else {
      followModeActive = false;
      obstacleModeActive = false;
    }
  }
}

void sendResponse() {
  if (followModeActive) {
    followMode();
  } else if (obstacleModeActive) {
    int distance = getDistance();
    Wire.write(distance);
  } else {
    Wire.write('S');
  }
  checkLight();
}

void followMode() {
  int leftIR = digitalRead(LEFT_IR);
  int rightIR = digitalRead(RIGHT_IR);
  int distance = getDistance();

  Serial.print("Left IR: ");
  Serial.print(leftIR);
  Serial.print(" | Right IR: ");
  Serial.print(rightIR);
  Serial.print(" | Distance: ");
  Serial.println(distance);

  char followCommand = 'S';

  if (leftIR == HIGH && rightIR == LOW) {
    followCommand = 'R';  // Turn Right
  } 
  else if (rightIR == HIGH && leftIR == LOW) {
    followCommand = 'L';  // Turn Left
  } 
  else if (distance < 120) {
    followCommand = 'U';  // Move Forward
  } 
  else {
    followCommand = 'S';  // Stop
  }

  Wire.write(followCommand);
}

void loop() {
  checkLight();
  if (digitalRead(TOUCH_SENSOR) == HIGH) {
    tailServo.write(60);
    delay(200);
    tailServo.write(120);
    delay(200);
  } else {
    tailServo.write(90);
  }
  delay(100);
}