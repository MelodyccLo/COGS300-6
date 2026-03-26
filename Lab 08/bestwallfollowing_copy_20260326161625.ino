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

// --- Tuned Settings ---
const int FRONT_STOP = 15;  // was 8 → stops before hitting wall
const int LEFT_OPEN = 35;   // was 20 → 30cm wall now correctly seen as blocked

const int baseSpeed = 90;
const int turnSpeedLeft = 120;
const int turnSpeedRight = 120;

const int turnDuration = 400;
const int backDuration = 250;

const int leftBoost = 0;
const int rightBoost = 25;  // was 15 → more compensation for weak right wheel

// --- Stuck detection ---
long lastFrontDist = 0;
unsigned long stuckTimer = 0;
const unsigned long stuckThreshold = 5000;
const int stuckTolerance = 3;

// ---------------- SETUP ----------------
void setup() {
  pinMode(enA, OUTPUT); pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);

  pinMode(trigFront, OUTPUT); pinMode(echoFront, INPUT);
  pinMode(trigLeft, OUTPUT); pinMode(echoLeft, INPUT);

  Serial.begin(9600);

  lastFrontDist = getDistanceAvg(trigFront, echoFront);
  stuckTimer = millis();
}

// ---------------- LOOP ----------------
void loop() {
  long FS = getDistanceAvg(trigFront, echoFront);
  long LS = getDistanceAvg(trigLeft, echoLeft);

  Serial.print("F="); Serial.print(FS);
  Serial.print(" L="); Serial.println(LS);

  // -------- STUCK DETECTION --------
  if (abs(FS - lastFrontDist) <= stuckTolerance) {
    if (millis() - stuckTimer > stuckThreshold) {
      Serial.println("Stuck detected → Back + Adjust");

      stopCar();
      delay(100);
      moveBackward(baseSpeed);
      delay(backDuration);
      stopCar();
      delay(100);

      LS = getDistanceAvg(trigLeft, echoLeft);
      if (LS > LEFT_OPEN) {
        Serial.println("Adjust left");
        turnLeft(turnSpeedLeft);
      } else {
        Serial.println("Adjust right");
        turnRight(turnSpeedRight);
      }
      delay(turnDuration);
      stopCar();
      delay(100);
      stuckTimer = millis();
      lastFrontDist = getDistanceAvg(trigFront, echoFront);
      return;
    }
  } else {
    stuckTimer = millis();
    lastFrontDist = FS;
  }

  // -------- WALL DETECTION --------
  if (FS <= FRONT_STOP) {
    Serial.println("Wall → stop + adjust");

    stopCar();
    delay(100);
    moveBackward(baseSpeed);
    delay(backDuration);
    stopCar();
    delay(100);

    LS = getDistanceAvg(trigLeft, echoLeft);
    if (LS > LEFT_OPEN) {
      Serial.println("Left open → LEFT");
      turnLeft(turnSpeedLeft);
    } else {
      Serial.println("Left blocked → RIGHT");
      turnRight(turnSpeedRight);
    }
    delay(turnDuration);
    stopCar();
    delay(100);
  }
  // -------- CLEAR PATH --------
  else {
    moveStraight(baseSpeed);
  }

  delay(50);
}

// ---------------- SENSOR FUNCTION ----------------
long getDistanceAvg(int trig, int echo) {
  long total = 0;
  for (int i = 0; i < 3; i++) {
    digitalWrite(trig, LOW); delayMicroseconds(2);
    digitalWrite(trig, HIGH); delayMicroseconds(10);
    digitalWrite(trig, LOW);

    long duration = pulseIn(echo, HIGH, 30000);
    long dist = (duration == 0) ? 999 : duration * 0.034 / 2;
    total += dist;
    delay(5);
  }
  return total / 3;
}

// ---------------- MOTOR HELPERS ----------------
int L(int spd) { return constrain(spd + leftBoost, 0, 255); }
int R(int spd) { return constrain(spd + rightBoost, 0, 255); }

void moveStraight(int spd) {
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  analogWrite(enB, R(spd));
  analogWrite(enA, L(spd));
}

void moveBackward(int spd) {
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  analogWrite(enB, R(spd));
  analogWrite(enA, L(spd));
}

void turnLeft(int spd) {
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  analogWrite(enB, R(spd));
  analogWrite(enA, L(spd));
}

void turnRight(int spd) {
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  analogWrite(enB, R(spd));
  analogWrite(enA, L(spd));
}

void stopCar() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
  analogWrite(enA, 0);
  analogWrite(enB, 0);
} 