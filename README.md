# MyRobot-V1 -- COGS 300 Robotics Project

An Arduino-based robot control system developed for COGS 300 (Designing and Understanding Cognitive Systems) at UBC. This project implements low-level motor control and serial diagnostics for a DC motor-driven robot, using an H-bridge driver and PWM-capable GPIO pins.

## Tech Stack

| Layer         | Technology                          |
|---------------|-------------------------------------|
| Language      | C++ (Arduino framework)             |
| Platform      | Arduino (AVR microcontroller)       |
| Motor Driver  | H-bridge (L298N or equivalent)      |
| Communication | UART Serial at 9600 baud            |
| Build Tool    | Arduino IDE / Arduino CLI           |

## Features

- **H-bridge motor control** -- Directional drive and stop functions via digital GPIO, with PWM-capable enable pin for speed modulation.
- **Modular sketch architecture** -- Separated concerns across multiple `.ino` files (main loop, motor logic, serial logging) for maintainability and clarity.
- **Timestamped serial logging** -- Tagged `[INFO]` and `[ERROR]` log output with millisecond timestamps over UART for real-time debugging and telemetry.
- **Extensible control loop** -- Clean main loop structure designed for adding custom drive routines, sensor integration, and behavior sequencing.

## Project Structure

```
src/
  MyRobot-V1/
    MyRobot-V1.ino   # Main sketch -- pin configuration, setup, and control loop
    Motor.ino         # H-bridge motor control helpers (drive, stop)
    Serial.ino        # Serial logging utilities (logInfo, logError)
docs/
  starter.md          # Getting started reference
```

## Getting Started

### Prerequisites

- [Arduino IDE](https://www.arduino.cc/en/software) (v2.x recommended) or [Arduino CLI](https://arduino.github.io/arduino-cli/)
- An Arduino board (Uno, Mega, or compatible)
- H-bridge motor driver (e.g., L298N) wired to pins 7, 8, and 9

### Upload

1. Clone the repository:
   ```bash
   git clone https://github.com/MelodyccLo/COGS300-6.git
   ```
2. Open `src/MyRobot-V1/MyRobot-V1.ino` in the Arduino IDE.
3. Select your board and port under **Tools**.
4. Click **Upload**.
5. Open the Serial Monitor (9600 baud) to view real-time log output.

### Pin Wiring Reference

| Arduino Pin | Function              | Motor Driver Pin |
|-------------|-----------------------|------------------|
| 9 (PWM)     | Enable A (speed)      | ENA              |
| 8           | Direction control 1   | IN1              |
| 7           | Direction control 2   | IN2              |

## Team

| Member  | Role           |
|---------|----------------|
| Keira   | Group 6 Member |
| Melody  | Group 6 Member |
| Regina  | Group 6 Member |

Built for COGS 300 at the University of British Columbia.
