const int pinLeft = 2;
const int pinRight = 4;

void setup() {
  pinMode(pinLeft, INPUT_PULLUP);   // use pullups; pressed/LOW = 0
  pinMode(pinRight, INPUT_PULLUP);
  Serial.begin(115200);
}

void loop() {
  int v1 = digitalRead(pinLeft);    // 0 or 1
  int v2 = digitalRead(pinRight);    // 0 or 1
  Serial.print(v1); Serial.print(',');
  Serial.println(v2);          // newline-terminated
  delay(5);                    // ~200 Hz (adjust as needed)
}