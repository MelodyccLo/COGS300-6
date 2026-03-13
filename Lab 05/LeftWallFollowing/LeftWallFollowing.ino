// --- Motor Pins ---
#define enA 11
#define enB 6
#define in1 13
#define in2 12
#define in3 8
#define in4 7

// --- Ultrasonic Pins ---
#define trigFront 9
#define echoFront 10
#define trigLeft 3 
#define echoLeft 4

// --- Constants ---
const int baseSpeed = 90;      // Overall speed
const float driftL = 2;  // How much the inner wheel slows down (0.6 = 70%)
const float driftS = 1.4;  // How much the inner wheel slows down (0.6 = 70%)
const float driftR = 1.2;
const int turnDuration = 500;   // For the emergency 90-degree turn
const int dist = 30;
const int tolerance = 5;
int t = 0;

void setup() {
  pinMode(enA, OUTPUT); pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);

  pinMode(trigFront, OUTPUT); pinMode(echoFront, INPUT);
  pinMode(trigLeft, OUTPUT); pinMode(echoLeft, INPUT);
  
  Serial.begin(9600);
}

void loop() {
  long FS = getDistance(trigFront, echoFront);
  long LS = getDistance(trigLeft, echoLeft);

  // --- START OF YOUR LOGIC ---
  
  if (FS > (dist + tolerance)) {
    // 1. Path ahead is clear: Follow Left Wall
    if (LS > 100) {
      Serial.println("EMERGENCY! Wall too far: Sharp Left Turn");
      stopCar();
      delay(100);
      if (t == 0) {
        turnLeft(baseSpeed); 
        delay(turnDuration);
        stopCar();
        t = t + 1;
      } else {
        driftLeft(baseSpeed);
      }
    } 
    else if (LS > (dist + tolerance) && LS <= 100) {
      Serial.println("Clear ahead - Too far from wall: Drifting Left");
      driftLeft(baseSpeed);
      t = 0;
    }
    else if (LS >= (dist - tolerance) && LS <= (dist + tolerance)) {
      Serial.println("Clear ahead - Sweet spot: Moving Straight");
      moveStraight(baseSpeed);
      t = 0;
    } 
    else {
      Serial.println("Clear ahead - Too close to wall: Drifting Right");
      driftRight(baseSpeed);
      t = 0;
    }
  } 
  else if (FS >= (dist - tolerance) && FS <= (dist + tolerance)) {
    // 2. Approaching Front Wall: Start escaping
    Serial.println("Wall approaching in front: Drifting Right");
    driftRight(baseSpeed);
  } 
  else { 
    // 3. Emergency (FS < 40): Sharp Turn
    Serial.println("EMERGENCY! Wall too close: Sharp Right Turn");
    stopCar();
    delay(100);
    turnRight(baseSpeed); 
    delay(turnDuration);
    stopCar();
  }
  
  delay(100); // Small pause for sensor stability
}

// --- Sensor Function ---
long getDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH, 30000); 
  if (duration == 0) return 999; 
  return duration * 0.034 / 2;
}

// --- Movement Functions ---
void moveStraight(int spd) {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); // Left Forward
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);  // Right Forward
  analogWrite(enA, spd * driftS);
  analogWrite(enB, spd);
}

void driftLeft(int spd) {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, spd * driftL); // Right motor speeds up
  analogWrite(enB, spd);               // Left motor full speed
}

void driftRight(int spd) {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, spd);               // Left motor full speed
  analogWrite(enB, spd * driftR); // Right motor slows down
}

void turnRight(int spd) {
  // Point turn: Left wheel forward, Right wheel backward
  digitalWrite(in1, HIGH);  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);  digitalWrite(in4, LOW); 
  analogWrite(enA, spd);   analogWrite(enB, spd);
}

void turnLeft(int spd) {
  // Point turn: Left wheel forward, Right wheel backward
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH); 
  analogWrite(enA, spd);   analogWrite(enB, spd);
}

void stopCar() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
  analogWrite(enA, 0);    analogWrite(enB, 0);
}
