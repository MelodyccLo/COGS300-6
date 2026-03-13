// --- Motor Pins based on your successful test ---
#define enA 11
#define enB 6
#define in1 13
#define in2 12
#define in3 8
#define in4 7

void setup() {
  Serial.begin(9600);
  
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  Serial.println("--- Car Console Control Ready ---");
  Serial.println("Commands: f (forward), b (back), l (left), r (right), s (stop)");
  Serial.println("Example: Type 'f200' and press Enter");
}

void loop() {
  if (Serial.available() > 0) {
    // Read the first character (the command)
    char cmd = Serial.read(); 
    
    // Read the rest as an integer (the speed)
    int spd = Serial.parseInt(); 

    // Visual feedback in the console
    Serial.print("Executing: ");
    Serial.print(cmd);
    Serial.print(" at speed ");
    Serial.println(spd);

    executeMove(cmd, spd);
  }
}

void executeMove(char dir, int spd) {
  switch(dir) {
    case 'f': driveForward(spd);  break;
    case 'b': driveBackward(spd); break;
    case 'l': turnLeft(spd);      break;
    case 'r': turnRight(spd);     break;
    case 's': stopCar();          break;
    default:  stopCar();          break; 
  }
}

void driveForward(int spd) {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, spd);   analogWrite(enB, spd);
}

void driveBackward(int spd) {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);
  analogWrite(enA, spd);   analogWrite(enB, spd);
}

void turnLeft(int spd) {
  digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);  digitalWrite(in4, HIGH);
  analogWrite(enA, spd);   analogWrite(enB, spd);
}

void turnRight(int spd) {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, spd);   analogWrite(enB, spd);
}

void stopCar() {
  digitalWrite(in1, LOW);  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);  digitalWrite(in4, LOW);
  analogWrite(enA, 0);     analogWrite(enB, 0);
}