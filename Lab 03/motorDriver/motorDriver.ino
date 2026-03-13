#define enA 11
#define enB 6
#define in1 13
#define in2 12
#define in3 8
#define in4 7

void setup() {
    // Set motor control pins as outputs
    pinMode(enA, OUTPUT);
    pinMode(enB, OUTPUT);
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
}

void loop() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    analogWrite(enA, 127); // Turn on motor at half power (PWM)
    analogWrite(enB, 127);
}