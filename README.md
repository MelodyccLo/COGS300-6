# COGS 300 Robotics -- Lab Portfolio

A collection of Arduino and Processing projects developed for COGS 300 (Designing and Understanding Cognitive Systems) at UBC. Covers progressive robotics topics from basic motor control through Bayesian object tracking and autonomous maze navigation.

## Tech Stack

| Layer          | Technology                                              |
|----------------|---------------------------------------------------------|
| Language       | C++ (Arduino), Processing (Java)                        |
| Platform       | Arduino Uno / Nano 33 BLE                               |
| Motor Control  | H-bridge (L298N), PWM speed modulation                  |
| Sensors        | Ultrasonic (HC-SR04), IR reflectance, rotary encoders, photoresistor |
| Communication  | Serial, WiFi/UDP, Bluetooth Low Energy (BLE GATT)       |
| Actuation      | DC motors, servo motor (180-degree pan)                  |
| Tools          | Arduino IDE, Processing IDE                              |

## Lab Overview

### Lab 03 -- Motor Control and Communication

PWM-based dual motor control with multiple command interfaces: serial console, WiFi/UDP remote control, and a Processing GUI with D-pad and speed slider.

| Sketch               | Description                                          |
|-----------------------|------------------------------------------------------|
| `motorDriver`         | Basic PWM motor control                              |
| `motorDriverNew`      | WiFi-enabled with UDP command receiver (port 8888)   |
| `motorDriverSimple`   | Serial console interface for keyboard control        |
| `motorDriverPanel.pde`| Processing GUI with D-pad and speed slider           |

### Lab 04 -- Sensor Integration and BLE

Encoder-based odometry with interrupt-driven pulse counting, BLE wireless control via GATT protocol, and photoresistor-based line detection.

| Sketch               | Description                                          |
|-----------------------|------------------------------------------------------|
| `encoderSimple`       | Basic digital encoder reading at 200 Hz              |
| `encoderMotor`        | Dual encoder + motor control with 115200 baud telemetry |
| `BLE_LED`             | BLE GATT service/characteristic setup                |
| `motorDriverBLE`      | BLE string commands for remote motor control         |
| `motorDriverBLEnew`   | BLE byte control + photoresistor tape detection      |
| `photocell`           | Photoresistor calibration sketch                     |
| `encoderSketch.pde`   | Processing CSV logger for encoder data               |

### Lab 05 -- Wall Following

Autonomous left-wall-following using dual ultrasonic sensors. Implements a state machine with proportional steering and drift correction to maintain constant wall distance.

| Sketch               | Description                                          |
|-----------------------|------------------------------------------------------|
| `LeftWallFollowing`   | Proportional wall-following with emergency evasion   |

### Lab 06 -- Line Following

Reactive line-following using dual IR reflectance sensors with binary thresholding. Includes drift-corrected variant with tuned timing for smoother trajectories.

| Sketch               | Description                                          |
|-----------------------|------------------------------------------------------|
| `lineFollowing`       | Standard line follower with symmetric behavior       |
| `lineFollowingDrift`  | Improved version with tuned drift coefficients       |

### Lab 07 -- Object Detection and Bayesian Tracking

Servo-mounted rotating ultrasonic sensor creates a 180-degree depth map. A Bayesian filter calculates object probability across scan angles using variance-based likelihood estimation and belief diffusion. Proportional steering drives the robot toward the highest-probability target.

| Sketch                 | Description                                          |
|------------------------|------------------------------------------------------|
| `servoPos`             | Servo position control via serial input              |
| `depthMap`             | 181-element depth array from full 180-degree sweep   |
| `objectTrackingBayes`  | Bayesian filter with prior, likelihood, and diffusion |
| `ObjectDetection`      | 3-point rapid scan with proportional steering        |
| `objectTrackingFinal`  | Production-ready 5-state FSM for full tracking cycle |

**Bayesian filter details:**
- Prior: P(object) = 0.5 uniform across all angles
- Likelihood: Variance threshold (30 cm) detects depth discontinuities -- high variance yields L(object) = 0.85
- Update: Standard Bayes rule with normalization
- Diffusion: 5% belief spreading to neighboring angles to account for object motion

### Lab 08 -- Maze Navigation

Maze solving using the right-hand wall-following heuristic with dual ultrasonic sensors and empirical motor asymmetry compensation.

| Sketch               | Description                                          |
|-----------------------|------------------------------------------------------|
| `mazeWall`            | Complete maze solver with motor drift correction     |

## Key Algorithms and Techniques

- **Bayesian inference** -- Probabilistic object detection with likelihood models and belief propagation
- **Proportional control** -- Continuous steering based on sensor error for wall following and object tracking
- **Interrupt service routines** -- Hardware interrupts for accurate encoder pulse counting
- **State machines** -- Multi-state FSMs for autonomous navigation behaviors
- **BLE GATT protocol** -- Custom service and characteristic UUIDs for wireless robot control
- **Sensor fusion** -- Combining ultrasonic, IR, encoder, and photoresistor data for decision-making

## Project Structure

```
COGS300-6/
├── src/MyRobot-V1/          # Base template (motor control + serial logging)
├── Lab 03/                  # Motor control, WiFi/UDP, Processing GUI
├── Lab 04/                  # Encoders, BLE, photoresistor, CSV logging
├── Lab 05/                  # Wall following with ultrasonic sensors
├── Lab 06/                  # Line following with IR sensors
├── Lab 07/                  # Bayesian object tracking with servo scanning
├── Lab 08/                  # Maze navigation
└── docs/                    # Getting started reference
```

## Getting Started

### Prerequisites

- [Arduino IDE](https://www.arduino.cc/en/software) (v2.x recommended)
- [Processing](https://processing.org/download) (for GUI sketches)
- Arduino board (Uno for most labs, Nano 33 BLE for Lab 04)

### Upload

1. Clone the repository:
   ```bash
   git clone https://github.com/MelodyccLo/COGS300-6.git
   ```
2. Open any `.ino` file in the Arduino IDE.
3. Select your board and port under **Tools**.
4. Click **Upload**.
5. For Processing sketches (`.pde`), open in Processing IDE and click **Run**.

## Team

| Member  | Role           |
|---------|----------------|
| Keira   | Group 6 Member |
| Melody  | Group 6 Member |
| Regina  | Group 6 Member |

Built for COGS 300 at the University of British Columbia.
