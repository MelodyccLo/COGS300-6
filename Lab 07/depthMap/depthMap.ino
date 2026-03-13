#include <Servo.h>


// Pin Definitions (Fill these in with your specific pins)
const int servoPin = 5;
const int trigPin  = 9;
const int echoPin  = 10;


// Constants
const int minAngle = 0;
const int maxAngle = 180;
const int scanDelay = 40; // Time for servo to physically move and settle (ms)


// The Depth Map Array
// Index 0-180 represents the degree, Value represents distance in cm
float depthMap[181];


Servo scannerServo;


void setup() {
  Serial.begin(115200);
 
  scannerServo.attach(servoPin);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
 
  Serial.println("Starting Sweep...");
}


void loop() {
  // Execute a full sweep from 0 to 180
  for (int angle = minAngle; angle <= maxAngle; angle++) {
    moveAndMeasure(angle);
  }
 
  // Optional: Print the array for debugging/Bayes Filter input
  printDepthMap();


  // Execute a return sweep to keep the map updated
  for (int angle = maxAngle; angle >= minAngle; angle--) {
    moveAndMeasure(angle);
  }
 
  printDepthMap();
}


void moveAndMeasure(int angle) {
  scannerServo.write(angle);
  delay(scanDelay); // Critical: lets the servo stop shaking
 
  depthMap[angle] = getDistance();
}


float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
 
  // Calculate distance: (duration / 2) * speed of sound (0.0343 cm/us)
  float distance = duration * 0.0343 / 2;
 
  // Handle out of range/timeout
  if (distance <= 0 || distance > 400) return 400.0;
  return distance;
}


void printDepthMap() {
  Serial.println("--- Current Depth Map ---");
  for (int i = 0; i <= 180; i++) {
    Serial.print(i);          // Print the angle
    Serial.print(" deg : ");  // Print the label
    Serial.print(depthMap[i]); // Print the distance
    Serial.println(" cm");    // Print the unit and move to a new line
  }
  Serial.println("-------------------------");
}

