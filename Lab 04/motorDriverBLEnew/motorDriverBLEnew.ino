#include <ArduinoBLE.h>

// --- Motor Pins ---
#define enA 11
#define enB 6
#define in1 13
#define in2 12
#define in3 8
#define in4 7

// --- Sensors & Constants ---
#define photocellPin A2
const int whiteTapeThreshold = 60; // Value > 60 means white tape detected
const int cruiseSpeed = 150;       // Fixed speed for all movements
const int turnDuration = 500;      // Milliseconds it takes to turn 90 degrees (adjust this!)

char currentCommand = 's'; // Tracks what the car is currently doing

BLEService carService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEByteCharacteristic controlChar("19B10001-E8F2-537E-4F6C-D104768A1214", BLEWrite | BLENotify);

void setup() {
  pinMode(enA, OUTPUT); pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);

  if (!BLE.begin()) { while (1); }

  BLE.setLocalName("Neil-SmartCar");
  BLE.setAdvertisedService(carService);
  carService.addCharacteristic(controlChar);
  BLE.addService(carService);
  BLE.advertise();
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    while (central.connected()) {
      // 1. Check for new BLE commands
      if (controlChar.written()) {
        currentCommand = (char)controlChar.value();
        handleNewCommand(currentCommand);
      }

      // 2. Continuous Logic for Forward/Backward
      if (currentCommand == 'f' || currentCommand == 'b') {
        int lightValue = analogRead(photocellPin);
        
        // Stop if we hit the white tape
        if (lightValue > whiteTapeThreshold) {
          stopCar();
          currentCommand = 's'; // Reset command so it doesn't keep trying to move
        }
      }
    }
    stopCar();
  }
}

void handleNewCommand(char cmd) {
  switch(cmd) {
    case 'f': driveForward(cruiseSpeed); break;
    case 'b': driveBackward(cruiseSpeed); break;
    case 's': stopCar(); break;
    
    case 'l': 
      turnLeft(cruiseSpeed);
      delay(turnDuration); // Moves for a set time to hit ~90 degrees
      stopCar();
      currentCommand = 's'; // Stop the loop logic
      break;

    case 'r': 
      turnRight(cruiseSpeed);
      delay(turnDuration);
      stopCar();
      currentCommand = 's';
      break;
  }
}

// --- Standard Movement Functions ---
void driveForward(int spd) {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, spd);   analogWrite(enB, spd);
}

void driveBackward(int spd) {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);
  analogWrite(enA, spd);   analogWrite(enB, spd);
}

void turnLeft(int spd) {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);
  analogWrite(enA, spd);   analogWrite(enB, spd);
}

void turnRight(int spd) {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, spd);   analogWrite(enB, spd);
}

void stopCar() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
  analogWrite(enA, 0);    analogWrite(enB, 0);
}