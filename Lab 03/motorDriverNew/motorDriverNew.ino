#include <WiFiS3.h>
#include <WiFiUdp.h>

// --- Updated Motor Pins ---
#define enA 11
#define enB 6
#define in1 13
#define in2 12
#define in3 8
#define in4 7

// --- WiFi Credentials ---
char ssid[] = "C SHARP"; // Change to your hotspot name
char pass[] = "11111111";     // Change to your hotspot password

int status = WL_IDLE_STATUS;
char packetBuffer[255]; 
WiFiUDP Udp;

void setup() {
  Serial.begin(9600);
  
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // Attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Connecting to: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(5000);
  }
  
  Serial.println("Connected!");
  printWifiStatus();
  Udp.begin(8888); // Listening on Port 8888
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = 0; 

    // Parse command (e.g., "f,200")
    char* command = strtok(packetBuffer, ",");
    char* speedVal = strtok(NULL, ",");
    
    if (command != NULL && speedVal != NULL) {
      char direction = command[0];
      int speed = atoi(speedVal);
      executeMove(direction, speed);
    }
  }
}

void executeMove(char dir, int spd) {
  switch(dir) {
    case 'f': driveForward(spd);  break;
    case 'b': driveBackward(spd); break;
    case 'l': turnLeft(spd);      break;
    case 'r': turnRight(spd);     break;
    case 's': stopCar();          break;
  }
}

// --- Directional Functions based on your new wiring ---

void driveForward(int spd) {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enA, spd);
  analogWrite(enB, spd);
}

void driveBackward(int spd) {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enA, spd);
  analogWrite(enB, spd);
}

void turnRight(int spd) {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enA, spd);
  analogWrite(enB, spd);
}

void turnLeft(int spd) {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enA, spd);
  analogWrite(enB, spd);
}

void stopCar() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  analogWrite(enA, 0);
  analogWrite(enB, 0);
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip); // IMPORTANT: Copy this IP to Processing!
}