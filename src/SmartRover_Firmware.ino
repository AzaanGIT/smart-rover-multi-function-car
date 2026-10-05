/*
 * Smart Rover: A Multi-Function Robotic Car
 * Operating Modes:
 *   1. Obstacle Avoiding (Autonomous Ultrasonic HC-SR04)
 *   2. Bluetooth Mobile Telemetry (HC-05 / HC-06)
 *   3. Serial Terminal Navigation
 *   4. Voice Control Command Interpretation
 *
 * Microcontroller: Arduino Uno (ATmega328P)
 * Motor Driver: L298N Dual H-Bridge Driver Module
 */

#include <SoftwareSerial.h>

// Ultrasonic Sensor HC-SR04 Pins
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;

// L298N Dual H-Bridge Motor Driver Control Pins
// Left Motors (OUT1 & OUT2)
const int IN1 = 5;
const int IN2 = 6;
// Right Motors (OUT3 & OUT4)
const int IN3 = 7;
const int IN4 = 8;

// Distance detection thresholds (in cm)
const int OBSTACLE_DISTANCE_THRESHOLD = 20; // Distance to stop and reroute
const int SAFE_DISTANCE_THRESHOLD     = 35; // Safe forward threshold

// Operational modes
enum Mode {
  OBSTACLE_AVOIDING = 0,
  BLUETOOTH_MANUAL  = 1,
  TERMINAL_CONTROL  = 2,
  VOICE_CONTROL     = 3
};

Mode currentMode = OBSTACLE_AVOIDING; // Default autonomous mode

void setup() {
  // Serial communication for Hardware Serial (Terminal / USB)
  Serial.begin(9600);

  // Initialize Ultrasonic Pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize Motor Control Pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Ensure motors start stopped
  stopRover();

  Serial.println(F("========================================"));
  Serial.println(F("   Smart Rover: Multi-Function Car"));
  Serial.println(F("   Arduino Uno + HC-SR04 + L298N + HC-05"));
  Serial.println(F("========================================"));
  Serial.println(F("Modes:"));
  Serial.println(F("  'O' -> Obstacle Avoidance"));
  Serial.println(F("  'M' -> Bluetooth Manual Mode"));
  Serial.println(F("  'T' -> Terminal Command Mode"));
  Serial.println(F("  'V' -> Voice Control Mode"));
  Serial.println(F("Movement Commands:"));
  Serial.println(F("  'F' = Forward, 'B' = Backward"));
  Serial.println(F("  'L' = Left,    'R' = Right, 'S' = Stop"));
}

// Function to measure distance in centimeters using HC-SR04
long getUltrasonicDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout (~5m max range)
  if (duration == 0) {
    return 999; // Out of range or no echo
  }
  return duration * 0.034 / 2; // Convert duration to distance in cm
}

// Motor Movement Functions
void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnLeft() {
  // Counter-rotate or single side drive
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnRight() {
  // Counter-rotate opposite
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopRover() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// Obstacle Avoidance Autonomous Logic
void runObstacleAvoidance() {
  long distance = getUltrasonicDistance();

  if (distance <= OBSTACLE_DISTANCE_THRESHOLD) {
    // Obstacle detected! Stop, back up slightly, and turn right to avoid
    stopRover();
    delay(200);

    moveBackward();
    delay(400);

    stopRover();
    delay(150);

    // Turn right to scan and route around barrier
    turnRight();
    delay(500);

    stopRover();
    delay(150);
  } else {
    // Path is clear: proceed forward
    moveForward();
  }
  delay(50);
}

// Handle movement commands received via Bluetooth, Voice App, or Terminal
void handleCommand(char cmd) {
  switch (cmd) {
    // Mode Switch Commands
    case 'O':
    case 'o':
      currentMode = OBSTACLE_AVOIDING;
      Serial.println(F("[Mode] Switched to Obstacle Avoidance"));
      break;
    case 'M':
    case 'm':
      currentMode = BLUETOOTH_MANUAL;
      stopRover();
      Serial.println(F("[Mode] Switched to Bluetooth Manual"));
      break;
    case 'T':
    case 't':
      currentMode = TERMINAL_CONTROL;
      stopRover();
      Serial.println(F("[Mode] Switched to Serial Terminal Control"));
      break;
    case 'V':
    case 'v':
      currentMode = VOICE_CONTROL;
      stopRover();
      Serial.println(F("[Mode] Switched to Voice Control"));
      break;

    // Movement Commands (Active across Manual, Voice, and Terminal modes)
    case 'F': // Forward / Voice "forward"
    case 'f':
      moveForward();
      break;
    case 'B': // Backward / Voice "backward"
    case 'b':
      moveBackward();
      break;
    case 'L': // Turn Left / Voice "left"
    case 'l':
      turnLeft();
      break;
    case 'R': // Turn Right / Voice "right"
    case 'r':
      turnRight();
      break;
    case 'S': // Stop / Voice "stop"
    case 's':
      stopRover();
      break;

    default:
      // Unknown command
      break;
  }
}

void loop() {
  // Check for incoming commands over Serial interface (Terminal or HC-05 connected via RX/TX)
  if (Serial.available() > 0) {
    char incomingByte = Serial.read();
    handleCommand(incomingByte);
  }

  // Execute behavior based on current active operational mode
  if (currentMode == OBSTACLE_AVOIDING) {
    runObstacleAvoidance();
  }
  // In BLUETOOTH_MANUAL, TERMINAL_CONTROL, and VOICE_CONTROL modes,
  // motors execute the state set by incoming commands until an 'S' or new command is received.
}
