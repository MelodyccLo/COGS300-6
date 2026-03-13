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
#define trigLeft  3
#define echoLeft  4

const int baseSpeed    = 100;
const int rightBoost   = 150;
const int turnDuration = 650;

void setup() {
  pinMode(enA, OUTPUT); pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);
  pinMode(trigFront, OUTPUT); pinMode(echoFront, INPUT);
  pinMode(trigLeft,  OUTPUT); pinMode(echoLeft,  INPUT);
  Serial.begin(9600);
}

void loop() {
  long FS = getDistance(trigFront, echoFront);
  long LS = getDistance(trigLeft,  echoLeft);

  Serial.print("F="); Serial.print(FS);
  Serial.print(" L="); Serial.println(LS);

  if (FS <= 30 && LS <= 40) {
    Serial.println("Front+Left wall → Turn RIGHT");
    stopCar(); delay(100);
    turnRight(baseSpeed);
    delay(turnDuration);
    stopCar(); delay(100);
  }
  else if (FS <= 30 && LS > 40) {
    Serial.println("Front wall, left open → Turn LEFT");
    stopCar(); delay(100);
    turnLeft(baseSpeed);
    delay(turnDuration);
    stopCar(); delay(100);
  }
  else {
    Serial.println("Clear → Straight");
    moveStraight(baseSpeed);
  }

  delay(100);
}

long getDistance(int trig, int echo) {
  digitalWrite(trig, LOW);  delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH, 30000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

int R(int spd) { return constrain(spd + rightBoost, 0, 255); }

void moveStraight(int spd) {
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  analogWrite(enB, spd);
  analogWrite(enA, R(spd));
}

void turnRight(int spd) {
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  analogWrite(enB, spd); analogWrite(enA, R(spd));
}

void turnLeft(int spd) {
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  analogWrite(enB, spd); analogWrite(enA, R(spd));
}

void stopCar() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
  analogWrite(enA, 0);    analogWrite(enB, 0);
}