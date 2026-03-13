#include <Servo.h>

// ─── Pin Definitions ────────────────────────────────────────────────────────
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

// ─── Tunable Constants ───────────────────────────────────────────────────────
const int   MIN_ANGLE          = 0;
const int   MAX_ANGLE          = 180;
const int   SCAN_DELAY         = 40;

const float MAX_DIST           = 200.0f;
const float TOUCH_DIST         = 10.0f;
const float WALL_DIST_FRONT    = 25.0f;

const float VARIANCE_THRESHOLD = 40.0f;
const float LIKELIHOOD_OBJECT  = 0.85f;
const float LIKELIHOOD_WALL    = 0.15f;
const float DIFFUSE_ALPHA      = 0.05f;
const float MIN_OBJECT_PROB    = 0.58f;

const int   SPEED_TURN         = 160;
const int   SPEED_DRIVE        = 180;
const int   TURN_MS_PER_DEG    = 4;    // ← calibrate this
const int   OBSTACLE_STOP_CM   = 12;

// ─── State Machine ───────────────────────────────────────────────────────────
enum RobotState {
  STATE_WAIT_AT_WALL,
  STATE_SCAN,
  STATE_TURN,
  STATE_APPROACH,
  STATE_DONE
};

RobotState robotState = STATE_WAIT_AT_WALL;

// ─── Global Data ─────────────────────────────────────────────────────────────
float depthMap[181];
float probability[181];
int   targetAngle    = -1;
float targetDistance = MAX_DIST;

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

  Serial.println("=== Bayes Object Tracker Ready ===");
}

// ════════════════════════════════════════════════════════════════════════════
void loop() {
  switch (robotState) {

    // ── 1. Confirm wall in front, then spin 180° to face the room ─────────
    case STATE_WAIT_AT_WALL: {
      float front = getFrontDist();
      Serial.print("[WAIT] Front dist: "); Serial.println(front);

      if (front < WALL_DIST_FRONT) {
        Serial.println(">> Wall confirmed. Turning 180° to face room...");
        delay(500);
        turnRight(180);      // ← spin away from wall
        delay(500);
        Serial.println(">> Facing room. Starting scan...");
        robotState = STATE_SCAN;
      } else {
        // Not close enough to wall yet — nudge forward
        driveForward();
        delay(100);
        stopMotors();
        delay(200);
      }
      break;
    }

    // ── 2. Bayes sweep ────────────────────────────────────────────────────
    case STATE_SCAN: {
      Serial.println("[SCAN] Sweeping 0→180...");
      for (int a = MIN_ANGLE; a <= MAX_ANGLE; a++) {
        moveAndMeasure(a);
      }
      updateBeliefs();
      diffuseBeliefs();

      Serial.println("[SCAN] Sweeping 180→0...");
      for (int a = MAX_ANGLE; a >= MIN_ANGLE; a--) {
        moveAndMeasure(a);
      }
      updateBeliefs();
      diffuseBeliefs();

      targetAngle = findTarget();
      if (targetAngle >= 0) {
        targetDistance = depthMap[targetAngle];
        Serial.print(">> Object found at ");
        Serial.print(targetAngle);
        Serial.print(" deg, ");
        Serial.print(targetDistance);
        Serial.println(" cm — turning...");
        robotState = STATE_TURN;
      } else {
        Serial.println(">> No object found. Rotating and re-scanning...");
        turnRight(20);
      }
      break;
    }

    // ── 3. Turn robot body toward target angle ────────────────────────────
    case STATE_TURN: {
      int offset = targetAngle - 90;
      Serial.print("[TURN] Offset from centre: "); Serial.println(offset);

      if (offset > 5) {
        turnRight(offset);
      } else if (offset < -5) {
        turnLeft(-offset);
      }

      scannerServo.write(90);
      delay(300);

      Serial.println(">> Aligned. Approaching...");
      robotState = STATE_APPROACH;
      break;
    }

    // ── 4. Drive straight toward object ──────────────────────────────────
    case STATE_APPROACH: {
      float front = getFrontDist();
      Serial.print("[APPROACH] Front dist: "); Serial.println(front);

      if (front <= TOUCH_DIST) {
        stopMotors();
        Serial.println(">> TOUCHED THE BIN! Mission complete.");
        robotState = STATE_DONE;
      } else if (front <= OBSTACLE_STOP_CM + 2) {
        driveForward();
        delay(80);
        stopMotors();
        delay(100);
      } else {
        float leftDist = getLeftDist();
        if (leftDist < 10.0f) {
          turnRight(5);
        }
        driveForward();
        delay(150);
        stopMotors();
        delay(50);
      }
      break;
    }

    // ── 5. Done ───────────────────────────────────────────────────────────
    case STATE_DONE: {
      stopMotors();
      Serial.println("[DONE] Object touched. Halting.");
      delay(10000);
      for (int i = 0; i <= MAX_ANGLE; i++) probability[i] = 0.5f;
      scannerServo.write(90);
      robotState = STATE_WAIT_AT_WALL;
      break;
    }
  }
}

// ════════════════════════════════════════════════════════════════════════════
//  SENSOR FUNCTIONS
// ════════════════════════════════════════════════════════════════════════════

void moveAndMeasure(int angle) {
  scannerServo.write(angle);
  delay(SCAN_DELAY);
  depthMap[angle] = getUltrasonicDist(TRIG_FRONT, ECHO_FRONT);
}

float getFrontDist() { return getUltrasonicDist(TRIG_FRONT, ECHO_FRONT); }
float getLeftDist()  { return getUltrasonicDist(TRIG_LEFT,  ECHO_LEFT);  }

float getUltrasonicDist(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  float dist = duration * 0.0343f / 2.0f;
  if (dist <= 0 || dist > MAX_DIST) return MAX_DIST;
  return dist;
}

// ════════════════════════════════════════════════════════════════════════════
//  BAYES FILTER
// ════════════════════════════════════════════════════════════════════════════

void updateBeliefs() {
  const int WINDOW = 5;
  for (int i = MIN_ANGLE; i <= MAX_ANGLE; i++) {
    float sum = 0, sumSq = 0;
    int count = 0;
    for (int j = i - WINDOW; j <= i + WINDOW; j++) {
      if (j < MIN_ANGLE || j > MAX_ANGLE) continue;
      float d = depthMap[j];
      sum += d; sumSq += d * d; count++;
    }
    float mean     = sum / count;
    float variance = (sumSq / count) - (mean * mean);
    bool  isBump   = (variance > VARIANCE_THRESHOLD) && (depthMap[i] < mean);
    float L        = isBump ? LIKELIHOOD_OBJECT : LIKELIHOOD_WALL;

    float pObj  = probability[i] * L;
    float pWall = (1.0f - probability[i]) * (1.0f - L);
    float norm  = pObj + pWall;
    if (norm > 1e-6f) probability[i] = pObj / norm;
    probability[i] = constrain(probability[i], 0.02f, 0.98f);
  }
}

void diffuseBeliefs() {
  float temp[181];
  for (int i = MIN_ANGLE; i <= MAX_ANGLE; i++) temp[i] = probability[i];
  for (int i = MIN_ANGLE; i <= MAX_ANGLE; i++) {
    float spread = DIFFUSE_ALPHA * temp[i];
    probability[i] -= spread;
    if (i > MIN_ANGLE) probability[i-1] += spread * 0.5f;
    if (i < MAX_ANGLE) probability[i+1] += spread * 0.5f;
    probability[i] = constrain(probability[i], 0.0f, 1.0f);
  }
}

int findTarget() {
  float bestScore = -1.0f;
  int   bestAngle = -1;
  for (int i = MIN_ANGLE; i <= MAX_ANGLE; i++) {
    if (depthMap[i] >= MAX_DIST)          continue;
    if (probability[i] < MIN_OBJECT_PROB) continue;
    float score = probability[i] * (1.0f - depthMap[i] / MAX_DIST);
    if (score > bestScore) { bestScore = score; bestAngle = i; }
  }
  return bestAngle;
}

// ════════════════════════════════════════════════════════════════════════════
//  MOTOR FUNCTIONS  (verified wiring: both HIGH = turn right)
// ════════════════════════════════════════════════════════════════════════════

void driveForward() {
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);   // left forward
  analogWrite(EN_LEFT, SPEED_DRIVE);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);  // right opposite → straight
  analogWrite(EN_RIGHT, SPEED_DRIVE);
}

void stopMotors() {
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); analogWrite(EN_LEFT,  0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); analogWrite(EN_RIGHT, 0);
}

void turnRight(int degrees) {
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(EN_LEFT, SPEED_TURN);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  analogWrite(EN_RIGHT, SPEED_TURN);
  delay(degrees * TURN_MS_PER_DEG);
  stopMotors();
  delay(100);
}

void turnLeft(int degrees) {
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  analogWrite(EN_LEFT, SPEED_TURN);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  analogWrite(EN_RIGHT, SPEED_TURN);
  delay(degrees * TURN_MS_PER_DEG);
  stopMotors();
  delay(100);
}