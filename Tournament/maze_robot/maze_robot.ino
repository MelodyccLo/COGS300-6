// ═══════════════════════════════════════════════════════════════════════════
//  Maze Tournament — Stage 1: Line Following → Stage 2: Wall Following
//
//  IR Sensor layout (front of robot):
//        [L: A1]   [M: A2]   [R: A0]
//
//  Transition trigger (one-way, never reverts):
//    All 3 IR sensors dark (no line) AND
//    LS < 70cm AND FS < 100cm for 5 consecutive readings
//
//  All motion uses a single unified motor layer with L/R boost correction.
// ═══════════════════════════════════════════════════════════════════════════

// ─── Motor Pins ──────────────────────────────────────────────────────────────
#define enA    11
#define enB     6
#define in1    13
#define in2    12
#define in3     8
#define in4     7

// ─── IR Sensor Pins ──────────────────────────────────────────────────────────
#define irRight A0
#define irLeft  A1
#define irMid   A2

// ─── Ultrasonic Pins ─────────────────────────────────────────────────────────
#define trigFront  9
#define echoFront 10
#define trigLeft   3
#define echoLeft   4

// ═══════════════════════════════════════════════════════════════════════════
//  GLOBAL STATE
// ═══════════════════════════════════════════════════════════════════════════
enum RobotMode { LINE_FOLLOWING, WALL_FOLLOWING };
RobotMode mode = LINE_FOLLOWING;

// ─── Transition detection ────────────────────────────────────────────────────
int   transitionCount    = 0;
const int TRANSITION_REQUIRED = 5;   // consecutive readings needed

// ═══════════════════════════════════════════════════════════════════════════
//  SHARED SPEED SETTINGS  (one place to tune everything)
// ═══════════════════════════════════════════════════════════════════════════
const int BASE_SPEED  = 130;  // straight driving — both stages
const int TURN_SPEED  = 130;  // all turns, snaps, recovery
const int BACK_SPEED  = 130;  // wall-follower reverse
const int leftBoost   = 0;    // per-wheel trim
const int rightBoost  = 25;   // compensates weak right wheel

// ═══════════════════════════════════════════════════════════════════════════
//  LINE FOLLOWER SETTINGS
// ═══════════════════════════════════════════════════════════════════════════
const int IR_THRESHOLD = 400;

const unsigned long SNAP_MS  = 100;
const unsigned long LUNGE_MS = 80;

int  lastTurn      = 1;       // -1 = left, 1 = right
bool inAllWhite    = false;
unsigned long allWhiteStart = 0;
const unsigned long INTERSECTION_THRESHOLD_MS = 100;

enum RecoverState { SNAPPING, LUNGING };
RecoverState recoverState = SNAPPING;
unsigned long recoverTimer = 0;
bool wasLost = false;

// ═══════════════════════════════════════════════════════════════════════════
//  WALL FOLLOWER SETTINGS
// ═══════════════════════════════════════════════════════════════════════════
const int FRONT_STOP   = 15;
const int LEFT_OPEN    = 35;
const int turnDuration = 400;
const int backDuration = 250;

long         lastFrontDist  = 0;
unsigned long stuckTimer    = 0;
const unsigned long stuckThreshold = 5000;
const int    stuckTolerance = 3;

// ═══════════════════════════════════════════════════════════════════════════
void setup() {
  pinMode(enA, OUTPUT); pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);

  pinMode(irRight, INPUT);
  pinMode(irLeft,  INPUT);
  pinMode(irMid,   INPUT);

  pinMode(trigFront, OUTPUT); pinMode(echoFront, INPUT);
  pinMode(trigLeft,  OUTPUT); pinMode(echoLeft,  INPUT);

  Serial.begin(9600);
  Serial.println("=== Maze Robot Ready — LINE mode ===");
}

// ═══════════════════════════════════════════════════════════════════════════
void loop() {
  if (mode == LINE_FOLLOWING) {
    runLineFollower();
  } else {
    runWallFollower();
  }
}

// ═══════════════════════════════════════════════════════════════════════════
//  TRANSITION CHECK
//  Call this only when all 3 IR sensors are dark (no line detected).
//  Returns true and flips mode when condition is confirmed.
// ═══════════════════════════════════════════════════════════════════════════
bool checkTransitionToWall() {
  long FS = getDistanceAvg(trigFront, echoFront);
  long LS = getDistanceAvg(trigLeft,  echoLeft);

  Serial.print("[TRANSITION?] F="); Serial.print(FS);
  Serial.print(" L="); Serial.print(LS);
  Serial.print(" count="); Serial.println(transitionCount);

  if (LS < 70 && FS < 100) {
    transitionCount++;
    if (transitionCount >= TRANSITION_REQUIRED) {
      Serial.println(">>> SWITCHING TO WALL FOLLOWING <<<");
      mode = WALL_FOLLOWING;
      // Seed wall-follower state so stuck-detection starts fresh
      lastFrontDist = FS;
      stuckTimer    = millis();
      return true;
    }
  } else {
    transitionCount = 0;   // reset — readings must be consecutive
  }
  return false;
}

// ═══════════════════════════════════════════════════════════════════════════
//  STAGE 1 — LINE FOLLOWER
// ═══════════════════════════════════════════════════════════════════════════
void runLineFollower() {
  bool L = (analogRead(irLeft)  < IR_THRESHOLD);
  bool M = (analogRead(irMid)   < IR_THRESHOLD);
  bool R = (analogRead(irRight) < IR_THRESHOLD);

  bool onLine = L || M || R;

  // ── Found the line — exit recovery ─────────────────────────────────────
  if (wasLost && onLine) {
    wasLost         = false;
    transitionCount = 0;   // back on line — reset any partial transition count
  }

  // ── No line detected — check for maze entry or run recovery ────────────
  if (!onLine) {
    // Check transition condition before doing recovery movement
    if (checkTransitionToWall()) return;

    if (!wasLost) {
      wasLost      = true;
      recoverState = SNAPPING;
      recoverTimer = millis();
    }

    unsigned long elapsed = millis() - recoverTimer;

    if (recoverState == SNAPPING) {
      if (lastTurn == -1) snapLeft(TURN_SPEED);
      else                snapRight(TURN_SPEED);
      if (elapsed >= SNAP_MS) {
        recoverState = LUNGING;
        recoverTimer = millis();
      }
    } else {
      moveStraight(BASE_SPEED);
      if (elapsed >= LUNGE_MS) {
        recoverState = SNAPPING;
        recoverTimer = millis();
      }
    }
    return;
  }

  // ── Normal line-following ───────────────────────────────────────────────
  if (L && M && R) {
    if (!inAllWhite) {
      inAllWhite    = true;
      allWhiteStart = millis();
    } else if (millis() - allWhiteStart >= INTERSECTION_THRESHOLD_MS) {
      snapRight(TURN_SPEED);
    } else {
      moveStraight(BASE_SPEED);
    }

  } else if (L && !R) {
    inAllWhite = false;
    lastTurn   = -1;
    snapLeft(TURN_SPEED);

  } else if (R && !L) {
    inAllWhite = false;
    lastTurn   = 1;
    snapRight(TURN_SPEED);

  } else {
    inAllWhite = false;
    moveStraight(BASE_SPEED);
  }
}

// ═══════════════════════════════════════════════════════════════════════════
//  STAGE 2 — WALL FOLLOWER
// ═══════════════════════════════════════════════════════════════════════════
void runWallFollower() {
  long FS = getDistanceAvg(trigFront, echoFront);
  long LS = getDistanceAvg(trigLeft,  echoLeft);

  Serial.print("F="); Serial.print(FS);
  Serial.print(" L="); Serial.println(LS);

  // ── Stuck detection ──────────────────────────────────────────────────────
  if (abs(FS - lastFrontDist) <= stuckTolerance) {
    if (millis() - stuckTimer > stuckThreshold) {
      Serial.println("Stuck → Back + Adjust");
      stopCar(); delay(100);
      moveBackward(BACK_SPEED); delay(backDuration);
      stopCar(); delay(100);

      LS = getDistanceAvg(trigLeft, echoLeft);
      if (LS > LEFT_OPEN) { Serial.println("Adjust left");  turnLeft(TURN_SPEED); }
      else                 { Serial.println("Adjust right"); turnRight(TURN_SPEED); }
      delay(turnDuration);
      stopCar(); delay(100);

      stuckTimer    = millis();
      lastFrontDist = getDistanceAvg(trigFront, echoFront);
      return;
    }
  } else {
    stuckTimer    = millis();
    lastFrontDist = FS;
  }

  // ── Wall ahead ───────────────────────────────────────────────────────────
  if (FS <= FRONT_STOP) {
    Serial.println("Wall → stop + adjust");
    stopCar(); delay(100);
    moveBackward(BACK_SPEED); delay(backDuration);
    stopCar(); delay(100);

    LS = getDistanceAvg(trigLeft, echoLeft);
    if (LS > LEFT_OPEN) { Serial.println("Left open → LEFT");    turnLeft(TURN_SPEED); }
    else                 { Serial.println("Left blocked → RIGHT"); turnRight(TURN_SPEED); }
    delay(turnDuration);
    stopCar(); delay(100);

  } else {
    moveStraight(BASE_SPEED);
  }

  delay(50);
}

// ═══════════════════════════════════════════════════════════════════════════
//  SENSOR HELPER
// ═══════════════════════════════════════════════════════════════════════════
long getDistanceAvg(int trig, int echo) {
  long total = 0;
  for (int i = 0; i < 3; i++) {
    digitalWrite(trig, LOW);  delayMicroseconds(2);
    digitalWrite(trig, HIGH); delayMicroseconds(10);
    digitalWrite(trig, LOW);
    long duration = pulseIn(echo, HIGH, 30000);
    total += (duration == 0) ? 999 : duration * 0.034 / 2;
    delay(5);
  }
  return total / 3;
}

// ═══════════════════════════════════════════════════════════════════════════
//  UNIFIED MOTOR HELPERS  (used by both stages)
// ═══════════════════════════════════════════════════════════════════════════
int Lspd(int spd) { return constrain(spd + leftBoost,  0, 255); }
int Rspd(int spd) { return constrain(spd + rightBoost, 0, 255); }

void moveStraight(int spd) {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);
  analogWrite(enA, Lspd(spd));
  analogWrite(enB, Rspd(spd));
}

void moveBackward(int spd) {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, Lspd(spd));
  analogWrite(enB, Rspd(spd));
}

void snapRight(int spd) {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, Lspd(spd)); analogWrite(enB, Rspd(spd));
}

void snapLeft(int spd) {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);
  analogWrite(enA, Lspd(spd)); analogWrite(enB, Rspd(spd));
}

// turnLeft / turnRight are aliases for snapLeft / snapRight (wall follower uses these names)
void turnLeft(int spd)  { snapLeft(spd);  }
void turnRight(int spd) { snapRight(spd); }

void stopCar() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
  analogWrite(enA, 0);    analogWrite(enB, 0);
}
