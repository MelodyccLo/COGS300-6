#include <Servo.h>

// ─── Pins ────────────────────────────────────────────────────────────────────
const int SERVO_PIN  = 5;
const int TRIG_FRONT = 9;
const int ECHO_FRONT = 10;
const int TRIG_LEFT  = 3;
const int ECHO_LEFT  = 4;
const int EN_LEFT    = 6;
const int IN3        = 8;
const int IN4        = 7;
const int EN_RIGHT   = 11;
const int IN2        = 12;
const int IN1        = 13;

// ─── Tunable ─────────────────────────────────────────────────────────────────
const float MAX_DIST        = 200.0f;
const float TOUCH_DIST      = 10.0f;
const float WALL_DIST_FRONT = 25.0f;
const int   SCAN_DELAY      = 25;    // faster scan since only 3 points

const int   BASE_SPEED      = 160;   // forward speed
const int   TURN_BOOST      = 60;    // how much to boost one side when steering
// Steering: fast side = BASE_SPEED + TURN_BOOST, slow side = BASE_SPEED - TURN_BOOST

const int   STEER_THRESHOLD = 10;    // cm difference to trigger steering
const float VARIANCE_THRESHOLD = 35.0f;

// Full scan constants
const int MIN_ANGLE = 0;
const int MAX_ANGLE = 180;
const int FULL_SCAN_DELAY = 40;

// ─── State Machine ────────────────────────────────────────────────────────────
enum RobotState {
  STATE_WAIT_AT_WALL,
  STATE_TURN_AROUND,
  STATE_FULL_SCAN,      // one-time full scan to find object
  STATE_TRACK           // continuous proportional steering toward object
};

RobotState robotState = STATE_WAIT_AT_WALL;

float depthMap[181];
float probability[181];
int   targetAngle  = -1;
float objectDist   = MAX_DIST;

Servo scannerServo;

// ════════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  scannerServo.attach(SERVO_PIN);
  pinMode(TRIG_FRONT, OUTPUT); pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_LEFT,  OUTPUT); pinMode(ECHO_LEFT,  INPUT);
  pinMode(EN_LEFT,  OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(EN_RIGHT, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  stopMotors();

  for (int i = 0; i <= MAX_ANGLE; i++) {
    probability[i] = 0.5f;
    depthMap[i]    = MAX_DIST;
  }
  scannerServo.write(90);
  delay(500);
  Serial.println("=== Proportional Steering Tracker Ready ===");
}

// ════════════════════════════════════════════════════════════════════════════
void loop() {
  switch (robotState) {

    case STATE_WAIT_AT_WALL: {
      float front = getFrontDist();
      Serial.print("[WAIT] "); Serial.println(front);
      if (front < WALL_DIST_FRONT) {
        robotState = STATE_TURN_AROUND;
      } else {
        driveForward(); delay(100); stopMotors(); delay(200);
      }
      break;
    }

    case STATE_TURN_AROUND: {
      Serial.println("[TURN] Spinning 180°...");
      stopMotors(); delay(500);
      turnRight(180);
      delay(500);
      robotState = STATE_FULL_SCAN;
      break;
    }

    // ── One-time full Bayes scan to locate object ─────────────────────────
    case STATE_FULL_SCAN: {
      Serial.println("[FULL SCAN] 0→180...");
      for (int i = 0; i <= MAX_ANGLE; i++) {
        probability[i] = 0.5f;
        depthMap[i]    = MAX_DIST;
      }
      for (int a = MIN_ANGLE; a <= MAX_ANGLE; a++) {
        scannerServo.write(a);
        delay(FULL_SCAN_DELAY);
        depthMap[a] = getUltrasonicDist(TRIG_FRONT, ECHO_FRONT);
      }
      updateBeliefs();
      diffuseBeliefs();

      targetAngle = findTarget();
      if (targetAngle >= 0) {
        objectDist = depthMap[targetAngle];
        Serial.print(">> Object at "); Serial.print(targetAngle);
        Serial.print("deg, ");         Serial.print(objectDist);
        Serial.println("cm — switching to tracking");

        // Do one coarse body turn so object is roughly in front
        int offset = targetAngle - 90;
        if      (offset >  15) turnRight(offset);
        else if (offset < -15) turnLeft(-offset);
        delay(400);

        scannerServo.write(90);
        delay(300);
        robotState = STATE_TRACK;
      } else {
        Serial.println(">> Not found, rotating...");
        turnRight(30);
        delay(300);
      }
      break;
    }

    // ── Continuous proportional steering ─────────────────────────────────
    case STATE_TRACK: {
      // 1. Quick 3-point scan: left / centre / right
      int   angles[3]   = {75, 90, 105};
      float readings[3];
      for (int i = 0; i < 3; i++) {
        scannerServo.write(angles[i]);
        delay(SCAN_DELAY);
        readings[i] = getUltrasonicDist(TRIG_FRONT, ECHO_FRONT);
      }
      // Return servo to centre
      scannerServo.write(90);

      float dLeft   = readings[0];  // 75°
      float dCentre = readings[1];  // 90°
      float dRight  = readings[2];  // 105°

      Serial.print("[TRACK] L="); Serial.print(dLeft);
      Serial.print(" C=");        Serial.print(dCentre);
      Serial.print(" R=");        Serial.println(dRight);

      // 2. Check for arrival
      if (dCentre <= TOUCH_DIST ||
          dLeft   <= TOUCH_DIST ||
          dRight  <= TOUCH_DIST) {
        stopMotors();
        Serial.println(">> TOUCHED THE BIN! Done.");
        robotState = STATE_WAIT_AT_WALL;  // reset for re-run
        delay(5000);
        break;
      }

      // 3. Find closest of the 3 — that's where the object is
      float minDist  = min(dLeft, min(dCentre, dRight));

      // 4. If everything is far and similar → lost object → re-scan
      if (minDist >= MAX_DIST * 0.95f) {
        Serial.println(">> Lost object — re-scanning");
        stopMotors();
        robotState = STATE_FULL_SCAN;
        break;
      }

      // 5. Proportional steering decision
      // Compute a steering error: negative = object left, positive = object right
      float steerError = dLeft - dRight;  // left closer → steerError negative → turn left

      int leftSpeed, rightSpeed;

      if (abs(steerError) < STEER_THRESHOLD) {
        // Object roughly centred — drive straight
        leftSpeed  = BASE_SPEED;
        rightSpeed = BASE_SPEED;
        Serial.println("  → Straight");
      } else if (steerError < 0) {
        // Left is closer — curve left
        // (slow down left motor, speed up right)
        leftSpeed  = BASE_SPEED - TURN_BOOST;
        rightSpeed = BASE_SPEED + TURN_BOOST;
        Serial.println("  → Curve LEFT");
      } else {
        // Right is closer — curve right
        leftSpeed  = BASE_SPEED + TURN_BOOST;
        rightSpeed = BASE_SPEED - TURN_BOOST;
        Serial.println("  → Curve RIGHT");
      }

      // 6. Apply speeds — robot moves continuously, no stopping to turn
      steerDrive(leftSpeed, rightSpeed);
      delay(120);  // short control loop interval

      break;
    }
  }
}

// ════════════════════════════════════════════════════════════════════════════
//  MOTOR FUNCTIONS
// ════════════════════════════════════════════════════════════════════════════

// Differential steering: each side gets its own speed
void steerDrive(int leftSpeed, int rightSpeed) {
  leftSpeed  = constrain(leftSpeed,  0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(EN_LEFT, leftSpeed);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  analogWrite(EN_RIGHT, rightSpeed);
}

void driveForward() {
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(EN_LEFT, BASE_SPEED);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  analogWrite(EN_RIGHT, BASE_SPEED);
}

void stopMotors() {
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); analogWrite(EN_LEFT,  0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); analogWrite(EN_RIGHT, 0);
}

void turnRight(int degrees) {
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(EN_LEFT, 160);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  analogWrite(EN_RIGHT, 160);
  delay(degrees * 4);
  stopMotors(); delay(100);
}

void turnLeft(int degrees) {
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  analogWrite(EN_LEFT, 160);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  analogWrite(EN_RIGHT, 160);
  delay(degrees * 4);
  stopMotors(); delay(100);
}

// ════════════════════════════════════════════════════════════════════════════
//  SENSORS
// ════════════════════════════════════════════════════════════════════════════

float getFrontDist() { return getUltrasonicDist(TRIG_FRONT, ECHO_FRONT); }
float getLeftDist()  { return getUltrasonicDist(TRIG_LEFT,  ECHO_LEFT);  }

float getUltrasonicDist(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  float dist = duration * 0.0343f / 2.0f;
  if (dist <= 0 || dist > MAX_DIST) return MAX_DIST;
  return dist;
}

// ════════════════════════════════════════════════════════════════════════════
//  BAYES (used only in full scan)
// ════════════════════════════════════════════════════════════════════════════

void updateBeliefs() {
  const int WINDOW = 5;
  for (int i = MIN_ANGLE; i <= MAX_ANGLE; i++) {
    float sum = 0, sumSq = 0; int count = 0;
    for (int j = i - WINDOW; j <= i + WINDOW; j++) {
      if (j < 0 || j > 180) continue;
      sum += depthMap[j]; sumSq += depthMap[j] * depthMap[j]; count++;
    }
    float mean = sum / count;
    float var  = (sumSq / count) - (mean * mean);
    bool  bump = (var > VARIANCE_THRESHOLD) && (depthMap[i] < mean);
    float L    = bump ? 0.85f : 0.15f;
    float pO   = probability[i] * L;
    float pW   = (1 - probability[i]) * (1 - L);
    float norm = pO + pW;
    if (norm > 1e-6f) probability[i] = pO / norm;
    probability[i] = constrain(probability[i], 0.02f, 0.98f);
  }
}

void diffuseBeliefs() {
  float temp[181];
  for (int i = 0; i <= 180; i++) temp[i] = probability[i];
  for (int i = 0; i <= 180; i++) {
    float s = 0.05f * temp[i];
    probability[i] -= s;
    if (i > 0)   probability[i-1] += s * 0.5f;
    if (i < 180) probability[i+1] += s * 0.5f;
    probability[i] = constrain(probability[i], 0.0f, 1.0f);
  }
}

int findTarget() {
  float best = -1; int bestA = -1;
  for (int i = 0; i <= 180; i++) {
    if (depthMap[i] >= MAX_DIST)   continue;
    if (probability[i] < 0.58f)    continue;
    float score = probability[i] * (1.0f - depthMap[i] / MAX_DIST);
    if (score > best) { best = score; bestA = i; }
  }
  return bestA;
}