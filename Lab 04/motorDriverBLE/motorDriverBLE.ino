#include <ArduinoBLE.h>

// --- Motor Pins ---
#define enA 11
#define enB 6
#define in1 13
#define in2 12
#define in3 8
#define in4 7

// BLE Service & Characteristic
// Using a "String" characteristic so we can send commands like "f255"
BLEService carService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEStringCharacteristic controlChar("19B10001-E8F2-537E-4F6C-D104768A1214", BLEWrite | BLENotify, 10);

void setup() {
  // Motors
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // Initialize BLE
  if (!BLE.begin()) {
    while (1); // Halt if BLE fails
  }

  BLE.setLocalName("Car"); // Your custom car name
  BLE.setAdvertisedService(carService);
  carService.addCharacteristic(controlChar);
  BLE.addService(carService);
  
  BLE.advertise();
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    while (central.connected()) {
      if (controlChar.written()) {
        String command = controlChar.value();
        
        // Split the string: first char is direction, rest is speed
        char dir = command.charAt(0);
        int spd = command.substring(1).toInt();

        executeMove(dir, spd);
      }
    }
    // Safety: Stop the car if the phone disconnects
    stopCar();
  }
}

// --- Motor Control Functions ---
void executeMove(char dir, int spd) {
  switch(dir) {
    case 'f': driveForward(spd);  break;
    case 'b': driveBackward(spd); break;
    case 'l': turnLeft(spd);      break;
    case 'r': turnRight(spd);     break;
    case 's': stopCar();          break;
    default:  stopCar();          break; 
  }
}

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
  digitalWrite(in1, LOW);  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);  digitalWrite(in4, LOW);
  analogWrite(enA, 0);     analogWrite(enB, 0);
}
