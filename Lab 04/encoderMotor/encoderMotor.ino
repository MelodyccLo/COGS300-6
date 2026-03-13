// --- Motor Pins ---
#define enA 11
#define enB 6
#define in1 13
#define in2 12
#define in3 8
#define in4 7

// --- Encoder Pins ---
const int encLeftD = 2;   // D0 of Left Encoder
const int encRightD = 4;  // D0 of Right Encoder
const int encLeftA = A1;  // A0 of Left Encoder
const int encRightA = A0; // A0 of Right Encoder

// Volatile is required for variables used in interrupts
volatile long leftTicks = 0;
long rightTicks = 0; 
int lastRightState = LOW;

void setup() {
  Serial.begin(115200); // Higher speed for better telemetry flow
  
  // Motor setup
  pinMode(enA, OUTPUT); pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);

  // Encoder setup
  pinMode(encLeftD, INPUT_PULLUP);
  pinMode(encRightD, INPUT_PULLUP);

  // Interrupt: Every time Pin 2 goes from LOW to HIGH, countLeft() runs automatically
  attachInterrupt(digitalPinToInterrupt(encLeftD), countLeft, RISING);

  Serial.println("STATUS:Ready,CMD_WAITING");
}

void countLeft() {
  leftTicks++; 
}

void loop() {
  // 1. Handle Motor Commands
  if (Serial.available() > 0) {
    char cmd = Serial.read(); 
    int spd = Serial.parseInt(); 
    executeMove(cmd, spd);
  }

  // 2. Read Right Encoder (Manual polling since Pin 4 isn't an interrupt pin on Uno)
  int currentRight = digitalRead(encRightD);
  if (currentRight == HIGH && lastRightState == LOW) {
    rightTicks++;
  }
  lastRightState = currentRight;

  // 3. Telemetry Output (Comma-Separated for Processing)
  // Format: LeftTicks, RightTicks, AnalogLeft, AnalogRight
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 50) { // 20Hz update rate
    Serial.print(leftTicks);  Serial.print(",");
    Serial.print(rightTicks); Serial.print(",");
    Serial.print(analogRead(encLeftA)); Serial.print(",");
    Serial.println(analogRead(encRightA));
    lastPrint = millis();
  }
}

// Reuse your executeMove, driveForward, and stopCar functions here...

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