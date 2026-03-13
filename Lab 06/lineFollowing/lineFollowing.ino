// --- Motor Pins ---
#define enA 11
#define enB 6
#define in1 13
#define in2 12
#define in3 8
#define in4 7

// --- IR Pins ---
#define irLeft A3
#define irRight A2

// --- Constants ---
const int cruiseSpeed = 60;    
const int turnSpeed = 65;      
const int irThreshold = 400;   // Low = White Tape (<400)
const float driftS = 1;      // Your hardware drift correction

void setup() {
  pinMode(enA, OUTPUT); pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);
  
  pinMode(irLeft, INPUT);
  pinMode(irRight, INPUT);

  Serial.begin(9600);
  Serial.println("System Ready: Line Following Active");
}

void loop() {
  // Read sensors
  bool leftOnTape = (analogRead(irLeft) < irThreshold);
  bool rightOnTape = (analogRead(irRight) < irThreshold);

  if (!leftOnTape && !rightOnTape) {
    // 1. Both on dark floor: Move Straight
    Serial.println("STR - Moving Straight");
    moveStraight(cruiseSpeed);
  } 
  else if (leftOnTape && !rightOnTape) {
    // 2. Left sensor hit tape: Nudge Left
    Serial.println("LEFT - Nudging Left");
    turnLeft(turnSpeed);
    delay(20); // Small fixed duration to clear the tape edge
  } 
  else if (rightOnTape && !leftOnTape) {
    // 3. Right sensor hit tape: Nudge Right
    Serial.println("RIGHT - Nudging Right");
    turnRight(turnSpeed);
    delay(20); 
  } 
  else {
    // 4. Both on tape: Possible Intersection or Stop
    Serial.println("STOP - Both sensors on tape");
    turnRight(turnSpeed);
    delay(20); // Small fixed duration to clear the tape edge
  }
}

// --- Movement Functions ---

void moveStraight(int spd) {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); 
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);  
  analogWrite(enA, spd); 
  analogWrite(enB, spd * driftS);
}

void turnRight(int spd) {
  // Point turn: Left wheel BACK, Right wheel FORWARD
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW); 
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW); 
  analogWrite(enA, spd);   analogWrite(enB, spd);
}

void turnLeft(int spd) {
  // Point turn: Left wheel FORWARD, Right wheel BACK
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);  
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);  
  analogWrite(enA, spd);   analogWrite(enB, spd);
}

void stopCar() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
  analogWrite(enA, 0);    analogWrite(enB, 0);
}