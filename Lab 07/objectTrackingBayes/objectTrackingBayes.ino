#include <Servo.h>

// ─── Pin Definitions ────────────────────────────────────────────────────────
const int SERVO_PIN = 5;
const int TRIG_PIN  = 9;
const int ECHO_PIN  = 10;

// ─── Scan Parameters ────────────────────────────────────────────────────────
const int MIN_ANGLE   = 0;
const int MAX_ANGLE   = 180;
const int SCAN_DELAY  = 40;   // ms – lets servo settle before reading

// ─── Bayes Filter Parameters ────────────────────────────────────────────────
// Prior / reset value (0.5 = completely uncertain)
const float PRIOR = 0.5f;

// How strongly a "bump" reading updates the probability
const float LIKELIHOOD_OBJECT = 0.85f;  // P(reading | object)
const float LIKELIHOOD_WALL   = 0.15f;  // P(reading | wall)

// Variance threshold: readings whose local depth variance exceeds this
// are classified as "object-like" rather than flat-wall-like.
const float VARIANCE_THRESHOLD = 30.0f; // cm²  (tune for your environment)

// Diffusion coefficient: how much probability spreads to neighbours each cycle
// Keeps the filter from becoming overconfident about a fixed location.
const float DIFFUSE_ALPHA = 0.05f;

// Maximum believable sensor reading
const float MAX_DIST = 400.0f;

// ─── Global State ────────────────────────────────────────────────────────────
float depthMap[181];       // raw distances (cm) for each degree
float probability[181];    // P(object) at each degree  [0 … 1]

Servo scannerServo;

// ════════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  scannerServo.attach(SERVO_PIN);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialise beliefs to the uniform prior
  for (int i = 0; i <= MAX_ANGLE; i++) {
    probability[i] = PRIOR;
    depthMap[i]    = MAX_DIST;
  }

  Serial.println("Bayes Object Tracker – ready.");
}

// ════════════════════════════════════════════════════════════════════════════
void loop() {
  // ── 1. Forward sweep: 0 → 180 ──────────────────────────────────────────
  for (int angle = MIN_ANGLE; angle <= MAX_ANGLE; angle++) {
    moveAndMeasure(angle);
  }

  // ── 2. Update beliefs with new depth data ───────────────────────────────
  updateBeliefs();

  // ── 3. Diffuse beliefs to account for possible object motion ────────────
  diffuseBeliefs();

  // ── 4. Find best target and report ─────────────────────────────────────
  int targetAngle = findTarget();
  reportResults(targetAngle);

  // ── 5. Return sweep: 180 → 0 (keeps map fresh) ─────────────────────────
  for (int angle = MAX_ANGLE; angle >= MIN_ANGLE; angle--) {
    moveAndMeasure(angle);
  }

  updateBeliefs();
  diffuseBeliefs();

  targetAngle = findTarget();
  reportResults(targetAngle);

  // ── 6. Point servo at best target ──────────────────────────────────────
  if (targetAngle >= 0) {
    scannerServo.write(targetAngle);
    // TODO: issue drive command toward targetAngle / depthMap[targetAngle]
  }
}

// ════════════════════════════════════════════════════════════════════════════
// Move servo to 'angle', wait for it to settle, then store a distance reading.
void moveAndMeasure(int angle) {
  scannerServo.write(angle);
  delay(SCAN_DELAY);
  depthMap[angle] = getDistance();
}

// ════════════════════════════════════════════════════════════════════════════
// Bayes update step.
//
// For each angle we compute a simple local variance over a ±5° window.
// A HIGH variance (depth "bump") → object-like → raises P(object).
// A LOW  variance (smooth wall)  → wall-like   → lowers  P(object).
//
// Bayes update:  P_new = P_old * L / normaliser
// where L = LIKELIHOOD_OBJECT if "bump", LIKELIHOOD_WALL otherwise.
void updateBeliefs() {
  const int WINDOW = 5; // half-width of local window in degrees

  for (int i = MIN_ANGLE; i <= MAX_ANGLE; i++) {
    // ── compute local variance ──────────────────────────────────────────
    float sum = 0, sumSq = 0;
    int   count = 0;
    for (int j = i - WINDOW; j <= i + WINDOW; j++) {
      if (j < MIN_ANGLE || j > MAX_ANGLE) continue;
      float d = depthMap[j];
      sum   += d;
      sumSq += d * d;
      count++;
    }
    float mean     = sum / count;
    float variance = (sumSq / count) - (mean * mean);

    // ── choose likelihood based on whether this looks like a bump ───────
    bool isBump = (variance > VARIANCE_THRESHOLD) &&
                  (depthMap[i] < mean);          // local minimum, not maximum

    float likelihood = isBump ? LIKELIHOOD_OBJECT : LIKELIHOOD_WALL;

    // ── Bayes update (log-odds form avoids float underflow) ──────────────
    // posterior ∝ likelihood × prior
    float pObj  = probability[i];
    float pWall = 1.0f - pObj;

    float newPObj  = pObj  * likelihood;
    float newPWall = pWall * (1.0f - likelihood);
    float norm     = newPObj + newPWall;

    if (norm > 1e-6f) {
      probability[i] = newPObj / norm;
    }
    // clamp to avoid lock-in
    probability[i] = constrain(probability[i], 0.02f, 0.98f);
  }
}

// ════════════════════════════════════════════════════════════════════════════
// Diffusion step: slightly spread probability mass to neighbours.
// Models the uncertainty that an object may have moved between sweeps.
void diffuseBeliefs() {
  float temp[181];
  for (int i = MIN_ANGLE; i <= MAX_ANGLE; i++) temp[i] = probability[i];

  for (int i = MIN_ANGLE; i <= MAX_ANGLE; i++) {
    float spread = DIFFUSE_ALPHA * temp[i];
    probability[i] -= spread;
    if (i > MIN_ANGLE) probability[i - 1] += spread * 0.5f;
    if (i < MAX_ANGLE) probability[i + 1] += spread * 0.5f;
    probability[i] = constrain(probability[i], 0.0f, 1.0f);
  }
}

// ════════════════════════════════════════════════════════════════════════════
// Find the angle with the highest P(object) that also has a finite distance.
// Returns -1 if nothing credible is found.
int findTarget() {
  float bestScore = -1.0f;
  int   bestAngle = -1;

  for (int i = MIN_ANGLE; i <= MAX_ANGLE; i++) {
    if (depthMap[i] >= MAX_DIST) continue;          // no echo / out of range
    if (probability[i] < 0.55f)  continue;          // not confident enough

    // Score = probability weighted by closeness
    // (closer objects score higher for equal probability)
    float distanceFactor = 1.0f - (depthMap[i] / MAX_DIST);
    float score = probability[i] * distanceFactor;

    if (score > bestScore) {
      bestScore = score;
      bestAngle = i;
    }
  }
  return bestAngle;
}

// ════════════════════════════════════════════════════════════════════════════
// Pretty-print results over Serial for debugging / visualisation.
void reportResults(int targetAngle) {
  Serial.println("=== Bayes Depth Map ===");
  Serial.println("Angle | Dist(cm) | P(obj) | Label");

  for (int i = MIN_ANGLE; i <= MAX_ANGLE; i += 5) { // print every 5° to save bandwidth
    Serial.print(i);
    Serial.print("\t");
    Serial.print(depthMap[i], 1);
    Serial.print("\t");
    Serial.print(probability[i], 3);
    Serial.print("\t");
    if (i == targetAngle)          Serial.print("<-- TARGET");
    else if (probability[i] > 0.7) Serial.print("object?");
    else if (probability[i] < 0.3) Serial.print("wall");
    Serial.println();
  }

  if (targetAngle >= 0) {
    Serial.print(">> TARGET: ");
    Serial.print(targetAngle);
    Serial.print(" deg, ");
    Serial.print(depthMap[targetAngle], 1);
    Serial.println(" cm");
  } else {
    Serial.println(">> No object found.");
  }
  Serial.println("=======================");
}

// ════════════════════════════════════════════════════════════════════════════
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30 ms timeout

  float dist = duration * 0.0343f / 2.0f;
  if (dist <= 0 || dist > MAX_DIST) return MAX_DIST;
  return dist;
}