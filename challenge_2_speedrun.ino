#include <Servo.h>

// --- Parallel L298N Motor Driver Pins ---
const int ENA = 6;   // PWM speed control (Connected to BOTH ENA & ENB)
const int IN1 = 3;   // Channel A Direction Pin 1
const int IN2 = 5;   // Channel A Direction Pin 2
const int IN3 = 11;  // Channel B Direction Pin 1
const int IN4 = 10;  // Channel B Direction Pin 2

// --- HC-SR04 Ultrasonic Sensor Pins ---
const int TRIG_PIN = A5;
const int ECHO_PIN = A4;

// --- Front Steering Servo Pin ---
const int STEER_SERVO_PIN = 9;
Servo steerServo;

// --- Steering Calibration ---
const int STEER_TRIM = 0; 
const int STEER_CENTER = 103 + STEER_TRIM; // Verified IRL straight angle

// --- Motor Speed & Distance Thresholds ---
const int MOTOR_SPEED = 255;        // 100% full power
const float STOP_DISTANCE_CM = 3.5; // Target stopping distance threshold

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

// Active electronic brake: brief reverse pulse to kill forward momentum
void activeBrake() {
  setMotorDirectionReverse();
  analogWrite(ENA, 200);
  delay(70); // 70ms pulse stops coasting inertia

  stopMotors();
}

void setup() {
  // Motor pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Ultrasonic pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Attach steering servo and lock straight at verified angle (103 deg)
  steerServo.attach(STEER_SERVO_PIN);
  steerServo.write(STEER_CENTER);

  Serial.begin(9600);
  delay(1000); // 1-second delay before departure
}

// Ultrasonic distance measurement with float precision
float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000); // 25ms timeout (~4m)

  // Disregard missed echoes or timeouts
  if (duration == 0) {
    return 999.0;
  }

  // Speed of sound: 0.0343 cm/us -> (duration * 0.0343) / 2
  return (float)duration * 0.0343 / 2.0;
}

// Drive forward at full power on both paralleled channels
void driveForward(int speed) {
  setMotorDirectionForward();
  analogWrite(ENA, speed);
}

void executeStopSequence() {
  // 1. Kill forward coasting motion instantly
  activeBrake();
  steerServo.write(STEER_CENTER); // Hold steering straight
  
  Serial.println("Target reached at 3.5 cm! Chassis braked.");
  Serial.println("Trial ended. Entering safe idle state.");

  while (true) {
    // Lock motors off permanently until manual reset button is pressed
    stopMotors();
  }
}

void loop() {
  float distance = readDistanceCM();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Trigger permanent stop once within 3.5 cm (above 1.8 cm deadband floor)
  if (distance <= STOP_DISTANCE_CM && distance > 1.8) {
    executeStopSequence();
  } else {
    driveForward(MOTOR_SPEED);
  }

  delay(50); // 50ms cycle prevents ultrasonic echo overlap
}
