
// These constants won't change. They're used to give names to the pins used:

const int analogInPin = A0;  // Analog input pin that the photoresistor is attached to

int sensorValue = 0;        // value read from the photoresistor

void setup() {

  // initialize serial communications at 9600 bps:

  Serial.begin(9600);
}

void loop() {

  // read the analog in value:
  sensorValue = analogRead(analogInPin);

  // print the results to the Serial Monitor:

  Serial.println(sensorValue);

  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:

  delay(200);
}
