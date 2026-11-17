# Autonomous-Robotic-Wheelchair-TBWC-

# TBWC Autonomous Control Firmware

Embedded C++ (Arduino) control systems and electronics integration for the **ENGG*1100 Teddy Bear Wheelchair (TBWC)** design project (Section 03, Team 04). 

This repository contains the autonomous navigation firmware, hardware pinout architecture, and calibration routines developed for the two competition events.

---

## Hardware & Wiring Architecture

* **Microcontroller:** Arduino Uno (ATmega328P)
* **Motor Driver:** Dual-Channel L298N H-Bridge (Paralleled Configuration)
* **Steering Actuator:** SG90 Micro-Servo (Front-axle Ackermann steering)
* **Distance Sensor:** HC-SR04 Ultrasonic Rangefinder

### Pinout Mapping

| Component | Module Pin | Arduino Pin | Notes |
| :--- | :--- | :--- | :--- |
| **L298N Speed (PWM)** | `ENA` + `ENB` | `D6` | Y-jumpered for synchronized PWM duty cycle |
| **L298N Channel A** | `IN1` / `IN2` | `D3` / `D5` | Paired with `OUT1` + `OUT3` (Motor Lead 1) |
| **L298N Channel B** | `IN3` / `IN4` | `D11` / `D10` | Paired with `OUT2` + `OUT4` (Motor Lead 2) |
| **Steering Servo** | Signal | `D9` | Calibrated mechanical center trim at `103°` |
| **HC-SR04 Ultrasonic** | `TRIG` / `ECHO` | `A5` / `A4` | Analog pins used as digital I/O |

### Key Hardware & Firmware Optimizations
* **Paralleled H-Bridge Current Doubling:** Tied L298N Channels A & B in parallel (`OUT1`+`OUT3`, `OUT2`+`OUT4`) with synchronized logic signals in software, doubling continuous current capacity from 2A to 4A (6A peak) to overcome loaded reverse stall torque and climb a 5 cm incline.
* **Servo Trim Calibration:** Compensated for discrete servo spline and linkage tolerances by calibrating software center to `103°` (`STEER_CENTER = 103`), eliminating open-loop directional drift.
* **Active Electronic Braking:** Implemented a 70 ms reverse-polarity torque pulse (`PWM 200`) to arrest forward coasting inertia immediately upon target detection.
* **Brownout & Back-EMF Protection:** Added a 300 ms voltage-settling pause and a stepped PWM soft-start ramp (`150` $\rightarrow$ `255`) during high-speed direction reversals to prevent supply voltage sag from resetting the microcontroller.

---

## Challenge Firmware Overview

### 1. Recycling Delivery Mission (`challenge_1_delivery`)
* **Objective:** Sprint autonomously toward the collection bin and halt without making physical contact (avoiding the collision penalty).
* **Control Logic:**
  * Drives forward at 100% PWM saturation (`255`) with front steering locked at `103°`.
  * Polls the HC-SR04 every 50 ms (`0.0343 cm/µs` speed-of-sound conversion).
  * Triggers `activeBrake()` once distance falls within **`3.5 cm`** (above a `1.8 cm` deadband floor to reject false echoes) and enters a permanent motor-lock idle state.

### 2. Accessibility Speed Run (`challenge_2_speedrun`)
* **Objective:** Climb a 5 cm ramp, cross **Line C**, and reverse back past the starting line in minimum time.
* **Course Geometry & Threshold Math:**
  * Arena tiles measure `60 cm` each; **Line C** is located on Tile 7 and the wooden backboard sits on Tile 9 (`120 cm` behind Line C).
  * Triggering turnaround at **`100.0 cm`** guarantees the front wheels fully clear Line C before reversing.
* **Control Logic:**
  * **Stage 1 (Outbound):** Full-power sprint with a `1.5 s` ultrasonic blanking lockout to ignore ramp/floor reflections, a 2-ping consecutive hit filter to reject acoustic noise, and a `4.2 s` backup timer failsafe.
  * **Stage 2 (Transition):** Executes active braking followed by a `300 ms` power-settling pause.
  * **Stage 3 (Return):** Soft-start PWM ramp into full-power reverse for `4.2 s` before final shutdown.
