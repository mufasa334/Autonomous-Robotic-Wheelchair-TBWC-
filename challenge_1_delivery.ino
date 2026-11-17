#include <Servo.h>

// --- Parallel L298N Motor Driver Pins ---
const int ENA = 6;   // Connected to BOTH ENA & ENB (PWM Speed Control)
const int IN1 = 3;   // Channel A Direction Pin 1
const int IN2 = 5;   // Channel A Direction Pin 2
const int IN3 = 11;  // Channel B Direction Pin 1
const int IN4 = 10;  // Channel B Direction Pin 2

// --- HC-SR04 Ultrasonic Sensor Pins ---
const int TRIG_PIN = A5;
const int ECHO_PIN = A4;

// --- Front Steering Servo ---
const int STEER_SERVO_PIN = 9;
Servo steerServo;
const int STEER_CENTER = 103; // Tested IRL straight angle

// --- Speed Run Performance Constants ---
const int SPRINT_SPEED = 255;           // 100% PWM power
const float BACKBOARD_REVERSE_CM = 100.0; // Trigger distance (100 cm)
const unsigned long OUTBOUND_TIMEOUT_MS = 4200; // 4.2s safety fallback timer
const unsigned long REVERSE_RUN_MS = 4200;      // 4.2s reverse duration

void setMotorDirectionForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void setMotorDirectionReverse() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
}

void activeBrake() {
  setMotorDirectionReverse();
  analogWrite(ENA, 200);
  delay(70);
  stopMotors();
}

float readBackboardDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 20000); // 20ms timeout

  if (duration == 0) {
    return 999.0;
  }

  return (float)duration * 0.0343 / 2.0;
}

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Lock steering straight at verified 103 degrees
  steerServo.attach(STEER_SERVO_PIN);
  steerServo.write(STEER_CENTER);

  Serial.begin(9600);
  delay(1000); // 1-second startup delay
}

void loop() {
  Serial.println("=== SPEED RUN LAUNCHED ===");
  unsigned long startTime = millis();
  bool reachedTurnaround = false;
  int hitCount = 0;

  // STAGE 1: FORWARD SPRINT
  setMotorDirectionForward();
  analogWrite(ENA, SPRINT_SPEED);

  while (!reachedTurnaround) {
    float distance = readBackboardDistance();
    unsigned long elapsedTime = millis() - startTime;

    // Ignore sensor readings during the first 1.5s
    if (elapsedTime > 1500) {
      if (distance <= BACKBOARD_REVERSE_CM && distance > 5.0) {
        hitCount++;
        if (hitCount >= 2) {
          Serial.println("Line C reached! Triggering reverse.");
          reachedTurnaround = true;
        }
      } else {
        hitCount = 0;
      }
    }

    // Safety timeout at 4.2s
    if (elapsedTime >= OUTBOUND_TIMEOUT_MS) {
      Serial.println("Backup timer triggered reverse.");
      reachedTurnaround = true;
    }

    delay(20);
  }

  // STAGE 2: BRAKE & SOFT REVERSE TRANSITION
  activeBrake();
  delay(300); // Brief pause to prevent voltage brownout/shock

  // STAGE 3: REVERSE SPRINT
  Serial.println("=== REVERSING TO FINISH LINE ===");
  setMotorDirectionReverse();

  // Soft-start ramp to protect driver/battery
  for (int pwm = 150; pwm <= SPRINT_SPEED; pwm += 35) {
    analogWrite(ENA, pwm);
    delay(15);
  }

  delay(REVERSE_RUN_MS);

  // STAGE 4: SHUTDOWN
  activeBrake();
  Serial.println("=== FINISHED RUN ===");

  while (true) {
    stopMotors();
  }
}
