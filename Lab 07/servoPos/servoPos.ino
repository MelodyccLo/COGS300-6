// C++ code
//
/*
  Servo Control via Serial Input

  by BARRAGAN <http://barraganstudio.com>
  This example code is in the public domain.

  modified with Serial Input for position control
*/

#include <Servo.h>

int pos = 0;
Servo servo_9;

void setup()
{
  // Start the serial communication at 9600 baud rate
  Serial.begin(9600);
  
  // Attach the servo on pin 9 to the servo object
  servo_9.attach(5, 500, 2500);
  
  // Print instructions for the user
  Serial.println("Enter a position between 0 and 180:");
}

void loop()
{
  // Check if there's any serial input available
  if (Serial.available() > 0) {
    // Read the input as a string
    String input = Serial.readStringUntil('\n');
    
    // Try to convert the input to an integer
    int newPos = input.toInt();

    // Check if the input is a valid number and within the range of 0-180
    if (newPos >= 0 && newPos <= 180) {
      // Move the servo to the new position
      servo_9.write(newPos);
      Serial.print("Moving to position: ");
      Serial.println(newPos);
    } else {
      // Print an error if the input is out of range or invalid
      Serial.println("Error: Please enter a valid position between 0 and 180.");
    }

    // Give the servo some time to move
    delay(500); // Adjust delay as needed
  }
}
