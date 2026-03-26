#define enA 11
#define enB 6
#define in1 13
#define in2 12
#define in3 8
#define in4 7

#define trigFront 9
#define echoFront 10
#define trigLeft  3
#define echoLeft  4

// --- Thresholds ---
const int FRONT_WALL   = 10;   // cm — wall ahead
const int LEFT_WALL    = 40;   // cm — left wall present (turn decision when front blocked)
const int LEFT_CLOSE   = 10;   // cm — too close to left wall (drift right)

// --- Speeds ---
const int baseSpeed      = 90;
const int turnSpeed      = 120;
const int driftAdjust    = 25;  // how much to reduce left motor when drifting right

const int leftBoost  = 0;
const int rightBoost = 15;  // tune this until moveStraight() goes straight

// --- Stuck detection ---
long lastFrontDist = 0;
long lastLeftDist  = 0;
unsigned long stuckTimer   = 0;
const unsigned long stuckThreshold = 5000;
const int stuckTolerance = 3;

// --- Turn timing ---
const int turnDuration = 400;
const int backDuration = 300;

void setup() {
  pinMode(enA, OUTPUT); pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);
  pinMode(trigFront, OUTPUT); pinMode(echoFront, INPUT);
  pinMode(trigLeft,  OUTPUT); pinMode(echoLeft,  INPUT);
  Serial.begin(9600);

  lastFrontDist = getDistanceAvg(trigFront, echoFront);
  lastLeftDist  = getDistanceAvg(trigLeft,  echoLeft);
  stuckTimer = millis();
}

void loop() {
  long FS = getDistanceAvg(trigFront, echoFront);
  long LS = getDistanceAvg(trigLeft,  echoLeft);

  Serial.print("F="); Serial.print(FS);
  Serial.print(" L="); Serial.println(LS);

  // -------- STUCK DETECTION (both sensors frozen) --------
  bool fsStuck = abs(FS - lastFrontDist) <= stuckTolerance;
  bool lsStuck = abs(LS - lastLeftDist)  <= stuckTolerance;

  if (fsStuck && lsStuck) {
    if (millis() - stuckTimer > stuckThreshold) {
      Serial.println("Stuck → backing up");
      stopCar();
      delay(100);
      moveBackward(baseSpeed);
      delay(backDuration);
      stopCar();
      delay(200);
      // reset timer and readings — let main logic take over after backup
      stuckTimer = millis();
      lastFrontDist = getDistanceAvg(trigFront, echoFront);
      lastLeftDist  = getDistanceAvg(trigLeft,  echoLeft);
      return;
    }
  } else {
    // something changed — not stuck
    stuckTimer = millis();
    lastFrontDist = FS;
    lastLeftDist  = LS;
  }

  // -------- MAIN LOGIC --------
  if (FS < FRONT_WALL) {
    // Wall ahead — decide turn direction using left sensor
    stopCar();
    delay(100);

    if (LS < LEFT_WALL) {
      // Left is also blocked — turn right to escape
      Serial.println("Front wall + left blocked → turn right");
      turnRight(turnSpeed);
    } else {
      // Left is open — turn left into open space
      Serial.println("Front wall + left open → turn left");
      turnLeft(turnSpeed);
    }

    delay(turnDuration);
    stopCar();
    delay(100);

  } else {
    // No wall ahead — manage left wall proximity
    if (LS < LEFT_CLOSE) {
      // Too close to left wall — drift right by slowing left motor
      Serial.println("Left wall close → drift right");
      driftRight();
    } else {
      // Clear path — drive straight
      moveStraight(baseSpeed);
    }
  }

  delay(50);
}

// -------- SENSOR --------
long getDistanceAvg(int trig, int echo) {
  long total = 0;
  for (int i = 0; i < 3; i++) {
    digitalWrite(trig, LOW);  delayMicroseconds(2);
    digitalWrite(trig, HIGH); delayMicroseconds(10);
    digitalWrite(trig, LOW);
    long duration = pulseIn(echo, HIGH, 30000);
    long dist = (duration == 0) ? 999 : duration * 0.034 / 2;
    total += dist;
    delay(5);
  }
  return total / 3;
}

// -------- MOTOR HELPERS --------
int L(int spd) { return constrain(spd + leftBoost,  0, 255); }
int R(int spd) { return constrain(spd + rightBoost, 0, 255); }

void moveStraight(int spd) {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);
  analogWrite(enA, L(spd));
  analogWrite(enB, R(spd));
}

void moveBackward(int spd) {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, L(spd));
  analogWrite(enB, R(spd));
}

void driftRight() {
  // Forward, but left motor slower than right → curves right
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);
  analogWrite(enA, L(baseSpeed - driftAdjust));  // slow left
  analogWrite(enB, R(baseSpeed));                // normal right
}

void turnLeft(int spd) {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);
  analogWrite(enA, L(spd));
  analogWrite(enB, R(spd));
}

void turnRight(int spd) {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, L(spd));
  analogWrite(enB, R(spd));
}

void stopCar() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
  analogWrite(enA, 0);
  analogWrite(enB, 0);
}