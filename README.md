# PIC16F877A Event Timer Project

## Project Description

This project is a programmable event timer designed and implemented using the PIC16F877A microcontroller. The system allows users to enter a countdown duration through a 4×4 matrix keypad and monitor the remaining time on a 16×2 LCD display. When the countdown reaches zero, an audible alarm is generated through a passive buzzer using PWM. The project also includes an emergency stop feature implemented with an external interrupt, which immediately stops the countdown and activates a warning siren.

In addition to the standard timer functionality, the system plays the *Pirates of the Caribbean* melody when the countdown is completed. The project was successfully tested in both Proteus simulation and real hardware implementation.

---

## Features

* 4×4 Matrix Keypad Input
* 16×2 LCD Display Interface
* Timer1-Based Countdown System
* PWM Audio Generation Using CCP1 Module
* Pirates of the Caribbean Melody Alarm
* Emergency Stop Function Using External Interrupt
* Emergency Warning Siren
* Proteus Simulation
* Real Hardware Implementation

---

## Hardware Components

* PIC16F877A Microcontroller
* PICkit 3 Programmer
* 20 MHz Crystal Oscillator
* 22 pF Capacitors
* 16×2 LCD Display
* 10 kΩ Potentiometer
* 4×4 Matrix Keypad
* Passive Buzzer
* Emergency Stop Push Button
* Breadboard
* Jumper Wires
* 5 V DC Power Supply

---

## Software Tools

* MPLAB X IDE
* XC8 Compiler
* Proteus Design Suite
* GitHub

---

## System Operation

1. The user enters a countdown value using the keypad.
2. The entered value is displayed on the LCD.
3. Timer1 interrupts maintain accurate countdown timing.
4. The remaining time is continuously updated on the LCD.
5. When the countdown reaches zero, the system plays the *Pirates of the Caribbean* melody through the buzzer.
6. If the emergency stop button is pressed, the external interrupt immediately stops the countdown and activates a warning siren.

---

## Team Members

* **Sude Filiz Diren**
* **Emre Ekici**

---

## Task Distribution

### Sude Filiz Diren

* LCD interface implementation
* Keypad scanning implementation
* Timer1 countdown logic
* Main program flow and user interface
* Software testing and debugging

### Emre Ekici

* PWM sound generation
* Pirates of the Caribbean melody implementation
* Emergency stop interrupt implementation
* Proteus simulation design
* Hardware assembly and testing
* Documentation and reporting

---

## Pin Connections

| Component             | PIC16F877A Pin           |
| --------------------- | ------------------------ |
| LCD Display           | PORTD                    |
| Keypad                | PORTB / PORTC            |
| Buzzer                | RC2 (CCP1 PWM Output)    |
| Emergency Stop Button | RB0 (External Interrupt) |
| Crystal Oscillator    | OSC1 / OSC2              |
| Power Supply          | VDD / VSS                |

---

## Project Structure

```text
PIC16F877A-Event-Timer/
│
├── README.md
├── Source_Code/
│   └── main.c
│
├── Proteus_Files/
│
├── Report/
│   └── Final_Report.pdf
│
├── Images/
│
└── Documentation/
```

---

## Test Results

| Function            | Status     |
| ------------------- | ---------- |
| Keypad Input        | Successful |
| LCD Display         | Successful |
| Timer1 Countdown    | Successful |
| PWM Audio Output    | Successful |
| Melody Playback     | Successful |
| Emergency Interrupt | Successful |
| Hardware Testing    | Successful |

---

## Submitted Files

The following materials were submitted separately through Microsoft Teams:

* Source code files
* Proteus simulation files
* Project report
* Project photographs
* Demonstration video (1–2 minutes)

---

## Project Outcome

The project successfully demonstrated the integration of several embedded-system concepts, including timer modules, interrupt handling, keypad interfacing, LCD communication, and PWM-based audio generation. The system operated reliably in both simulation and hardware environments and achieved all intended project objectives.

---

## References

1. Microchip Technology Inc. PIC16F87XA Data Sheet.
2. MPLAB X IDE Documentation.
3. XC8 Compiler Documentation.
4. HD44780 LCD Controller Documentation.
