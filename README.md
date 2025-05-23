# Arduino Smart Lock System

A secure smart lock system built with Arduino, using a state machine architecture. The system allows password entry via an IR remote, provides feedback via an LCD and buzzer, and includes features like proximity activation, password management, and lockout security.

## Features

- Lock/unlock mechanism controlled by a servo motor
- Proximity detection using an ultrasonic sensor (SR04)
- Password entry using an IR remote control
- LCD display for user interaction
- Password change via the Serial Monitor
- Lockout after multiple failed attempts
- Organized using a finite state machine design

## Components

- Arduino Uno or Nano
- IR receiver and IR remote
- SR04 ultrasonic distance sensor
- 16x2 LCD display
- Servo motor
- Piezo buzzer
- Red and green LEDs
- Jumper wires, resistors, breadboard

## IR Remote Mapping

| Button   | Action                        |
|----------|-------------------------------|
| Power    | Toggle system power           |
| OK       | Submit password               |
| 0–9      | Enter digits                  |
| Play     | Lock or unlock                |
| Function | Change password (after auth)  |

## System States

- **IDLE**: Waiting for a user to approach
- **AWAITING_PASSWORD**: User is entering the password
- **PASSWORD_SUCCESS**: Password is correct
- **LOCK_CONTROL**: User chooses to lock or unlock
- **PASSWORD_CHANGE**: User inputs a new password via Serial
- **LOCKED_OUT**: Temporary lockout after too many failed attempts

## How to Use

1. Power on the system using the IR remote.
2. Approach the device (within 50 cm) to activate the password prompt.
3. Use the IR remote to enter the password and press OK.
4. If the password is correct, select an action (lock/unlock or change password).
5. After 5 failed attempts, the system will lock out for 10 seconds.

## Serial Monitor (for password changes)

- Open the Serial Monitor at 9600 baud.
- When prompted, enter a new 4-digit password.
- Passwords longer or shorter than 4 characters are rejected.

## Project Structure

SmartLock/
├── SmartLock.ino   # Main Arduino source file
├── README.md       # Project documentation

## Dependencies

- Arduino IRremote library
- LiquidCrystal library
- Servo library
- SR04 ultrasonic sensor library (or custom implementation)

## License

This project is open-source and available under the MIT License.

## Author

Sam Marandi — Project created as part of a study in embedded systems and Arduino development.
