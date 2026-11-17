# TBWC // Autonomous Control Firmware 🦾

> **Embedded C++ Navigation & Motor Control for an Autonomous Mobility Rover**  
> *Built for ENGG\*1100 Engineering & Design I — Section 03, Team 04*

![Platform](https://img.shields.io/badge/Platform-Arduino%20Uno-00979D?logo=arduino&logoColor=white) ![Language](https://img.shields.io/badge/Language-C%2B%2B-00599C?logo=cplusplus&logoColor=white) ![Driver](https://img.shields.io/badge/Driver-Paralleled%20L298N-red) ![Sensor](https://img.shields.io/badge/Sensor-HC--SR04%20Ultrasonic-purple)

## 📖 Overview

**TBWC Autonomous Control Firmware** powers the autonomous navigation, drivetrain torque delivery, and collision-free braking systems for our **Teddy Bear Wheelchair (TBWC)** prototype.

Operating completely open-loop for trajectory with closed-loop ultrasonic rangefinding, the firmware coordinates a paralleled dual-channel L298N H-bridge, calibrated front-axle servo steering, and active reverse-polarity braking to handle both precision obstacle stops and high-speed ramp reversals without voltage brownouts.

## ⚡ Key Engineering Optimizations

- **🔌 Paralleled H-Bridge Current Doubling**: Tied L298N Channels A & B in parallel (`OUT1`+`OUT3`, `OUT2`+`OUT4`) with synchronized logic signals in software, doubling continuous current capacity from 2A to 4A (6A peak) to climb a 5 cm incline and overcome loaded reverse stall torque.
- **🛑 Active Electronic Braking**: Executes a `70 ms` reverse-polarity torque pulse (`PWM 200`) upon target detection to kill forward coasting inertia before locking the motor coils.
- **🔋 Brownout & Back-EMF Protection**: Enforces a `300 ms` voltage-settling pause and a stepped PWM soft-start ramp (`150` $\rightarrow$ `255`) during high-speed direction reversals to prevent supply voltage sag from resetting the ATmega328P.
- **🎯 Calibrated Servo Center Trim**: Compensates for mechanical linkage and spline tolerances by locking front Ackermann steering to `103°` (`STEER_CENTER = 103`), preventing directional drift across multi-tile sprints.
- **📡 Acoustic Noise & Ramp Filtering**: Combines a `1.5 s` launch blanking lockout, a `1.8 cm` deadband floor, and a 2-ping consecutive hit filter to eliminate false ultrasonic triggers from ramp reflections.

## 🛠️ Hardware & Pinout Architecture

- **Microcontroller**: Arduino Uno (ATmega328P)
- **Motor Driver**: Dual-Channel L298N H-Bridge (Paralleled Configuration)
- **Steering Actuator**: SG90 Micro-Servo
- **Distance Sensor**: HC-SR04 Ultrasonic Rangefinder
- **Libraries**: `Servo.h`

| Component | Module Pin | Arduino Pin | Configuration Notes |
| :--- | :--- | :--- | :--- |
| **L298N Speed (PWM)** | `ENA` + `ENB` | `D6` | Y-jumpered for synchronized PWM duty cycle |
| **L298N Channel A** | `IN1` / `IN2` | `D3` / `D5` | Paired with `OUT1` + `OUT3` (Motor Lead 1) |
| **L298N Channel B** | `IN3` / `IN4` | `D11` / `D10` | Paired with `OUT2` + `OUT4` (Motor Lead 2) |
| **Steering Servo** | Signal | `D9` | Calibrated mechanical center trim at `103°` |
| **HC-SR04 Ultrasonic** | `TRIG` / `ECHO` | `A5` / `A4` | Analog pins configured as digital I/O |

## 🏁 Challenge Firmware Breakdown

### 1. Recycling Delivery Mission (`challenge_1_delivery.ino`)
- **Goal**: Sprint autonomously toward the collection bin and halt without making physical contact (avoiding the collision penalty).
- **Control Sequence**:
  - Launches at 100% PWM saturation (`255`) with steering locked at `103°`.
  - Polls the HC-SR04 every `50 ms` (`0.0343 cm/µs` speed-of-sound conversion).
  - Triggers `activeBrake()` once distance falls within **`3.5 cm`** (above the `1.8 cm` deadband floor) and enters a permanent motor-lock idle loop.

### 2. Accessibility Speed Run (`challenge_2_speedrun.ino`)
- **Goal**: Climb a 5 cm ramp, clear **Line C**, and reverse back past the starting line in minimum time.
- **Course Geometry Math**: Arena floor tiles measure `60 cm` each; **Line C** sits on Tile 7 and the wooden backboard sits on Tile 9 (`120 cm` behind Line C). Setting the turnaround threshold to **`100.0 cm`** guarantees the front wheels fully cross Line C before reversing.
- **Control Sequence**:
  1. **Outbound Sprint**: Full-power forward drive with a `1.5 s` ultrasonic lockout, 2-ping confirmation filter, and `4.2 s` timer failsafe.
  2. **Turnaround Transition**: Active electronic brake followed by a `300 ms` power-rail settling delay.
  3. **Return Sprint**: Stepped PWM soft-start ramp (`150` $\rightarrow$ `255`) into full-power reverse for `4.2 s` before final shutdown.

## 🚀 Setup & Flashing

### Prerequisites
1. **Arduino IDE** (1.8.x or 2.x) with the built-in `Servo` library.
2. **USB-B Cable** connected to an **Arduino Uno**.

### Step 1: Clone the Repository
```bash
git clone [https://github.com/mufasa334/Autonomous-Robotic-Wheelchair-TBWC-.git](https://github.com/mufasa334/Autonomous-Robotic-Wheelchair-TBWC-.git)
cd Autonomous-Robotic-Wheelchair-TBWC-
```

### Step 2: Upload Challenge Firmware
1. Open `challenge_1_delivery.ino` (Recycling Mission) or `challenge_2_speedrun.ino` (Speed Run) in **Arduino IDE**.
2. Select **Tools → Board → Arduino Uno** and choose your active **COM / Serial Port**.
3. Click **Upload (`→`)**.
4. Place the chassis on the starting line and press the Arduino **RESET** button—the rover holds for a `1.0 s` startup delay before launching.

## ⚠️ Troubleshooting

- **Microcontroller resets during reverse transition**: Ensure battery pack voltage is above `7.5V` under load and verify the `300 ms` delay and PWM ramp in `challenge_2_speedrun.ino` are intact.
- **Rover veers left or right**: Adjust `STEER_TRIM` (or `STEER_CENTER = 103`) in increments of `±2°` to account for surface friction or linkage play.
- **Premature braking on launch**: Verify the HC-SR04 is tilted slightly upward or level so floor reflections do not trigger the `3.5 cm` threshold.
