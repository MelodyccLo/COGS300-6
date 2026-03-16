#include <Servo.h>

Servo scanner;

// PINS
const int SERVO_PIN = 5;

const int TRIG = 9;
const int ECHO = 10;

const int EN_LEFT = 6;
const int IN3 = 8;
const int IN4 = 7;

const int EN_RIGHT = 11;
const int IN1 = 13;
const int IN2 = 12;


// SCAN SETTINGS
const int MIN_ANGLE = 20;
const int MAX_ANGLE = 160;
const int STEP = 15;

const int SCAN_DELAY = 40;


// DISTANCES
const float WALL_DIST = 20;
const float TOUCH_DIST = 15;
const float MAX_DIST = 150;


// MOTOR
const int SPEED_DRIVE = 180;
const int SPEED_TURN = 160;

const int TURN_MS_PER_DEG = 4;


// GLOBALS
float bestDistance;
int bestAngle;


// STATES
enum RobotState {
  TURN_AROUND,
  SCAN,
  TURN_TO_OBJECT,
  DRIVE_TO_OBJECT
};

RobotState state = TURN_AROUND;


// ─────────────────────
void setup(){

Serial.begin(115200);

scanner.attach(SERVO_PIN);
scanner.write(90);

pinMode(TRIG,OUTPUT);
pinMode(ECHO,INPUT);

pinMode(EN_LEFT,OUTPUT);
pinMode(IN3,OUTPUT);
pinMode(IN4,OUTPUT);

pinMode(EN_RIGHT,OUTPUT);
pinMode(IN1,OUTPUT);
pinMode(IN2,OUTPUT);

stopMotors();

}


// ─────────────────────
void loop(){

switch(state){

// FIRST ACTION: TURN AWAY FROM WALL
case TURN_AROUND:

if(getDistance() < WALL_DIST){

Serial.println("Wall detected — turning around");

turnRight(180);

delay(500);

state = SCAN;

}

else{

state = SCAN;

}

break;


// SCAN ROOM
case SCAN:

scanRoom();

if(bestAngle != -1){

state = TURN_TO_OBJECT;

}

else{

turnRight(30);

}

break;


// TURN TOWARD OBJECT
case TURN_TO_OBJECT:

turnTo(bestAngle);

state = DRIVE_TO_OBJECT;

break;


// DRIVE TO OBJECT
case DRIVE_TO_OBJECT:

driveToObject();

state = SCAN;

break;

}

}


// ─────────────────────
// SCAN
// ─────────────────────

void scanRoom(){

bestDistance = MAX_DIST;
bestAngle = -1;

for(int angle = MIN_ANGLE; angle <= MAX_ANGLE; angle += STEP){

scanner.write(angle);

delay(SCAN_DELAY);

float d = getDistance();

if(d < bestDistance){

bestDistance = d;
bestAngle = angle;

}

}

scanner.write(90);

}


// ─────────────────────
// TURN TO TARGET
// ─────────────────────

void turnTo(int angle){

int offset = angle - 90;

if(offset > 0){

turnRight(offset);

}

else{

turnLeft(-offset);

}

}


// ─────────────────────
// DRIVE TO OBJECT
// ─────────────────────

void driveToObject(){

while(true){

float d = getDistance();

Serial.println(d);

if(d < TOUCH_DIST){

stopMotors();

Serial.println("OBJECT REACHED");

delay(3000);

return;

}

driveForward();

delay(120);

stopMotors();

}

}


// ─────────────────────
// ULTRASONIC
// ─────────────────────

float getDistance(){

float total = 0;

for(int i=0;i<3;i++){

digitalWrite(TRIG,LOW);
delayMicroseconds(2);

digitalWrite(TRIG,HIGH);
delayMicroseconds(10);
digitalWrite(TRIG,LOW);

long duration = pulseIn(ECHO,HIGH,30000);

float dist = duration*0.0343/2;

total += dist;

delay(5);

}

return total/3;

}


// ─────────────────────
// MOTORS
// ─────────────────────

void driveForward(){

digitalWrite(IN3,HIGH);
digitalWrite(IN4,LOW);

digitalWrite(IN1,LOW);
digitalWrite(IN2,HIGH);

analogWrite(EN_LEFT,SPEED_DRIVE);
analogWrite(EN_RIGHT,SPEED_DRIVE);

}


void stopMotors(){

digitalWrite(IN3,LOW);
digitalWrite(IN4,LOW);

digitalWrite(IN1,LOW);
digitalWrite(IN2,LOW);

analogWrite(EN_LEFT,0);
analogWrite(EN_RIGHT,0);

}


void turnRight(int deg){

digitalWrite(IN3,HIGH);
digitalWrite(IN4,LOW);

digitalWrite(IN1,HIGH);
digitalWrite(IN2,LOW);

analogWrite(EN_LEFT,SPEED_TURN);
analogWrite(EN_RIGHT,SPEED_TURN);

delay(deg*TURN_MS_PER_DEG);

stopMotors();
delay(100);

}


void turnLeft(int deg){

digitalWrite(IN3,LOW);
digitalWrite(IN4,HIGH);

digitalWrite(IN1,LOW);
digitalWrite(IN2,HIGH);

analogWrite(EN_LEFT,SPEED_TURN);
analogWrite(EN_RIGHT,SPEED_TURN);

delay(deg*TURN_MS_PER_DEG);

stopMotors();
delay(100);

}