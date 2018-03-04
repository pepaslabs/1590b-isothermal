#include "PID_v1.h"
#include "SoftwareSerial.h"

// --- user-tweakable values:

#define RX 3 // PB3, attiny pin 2
#define TX 4 // PB4, attiny pin 3
#define INPUT_PIN A0 // PB2, attiny pin 7
#define OUTPUT_PIN 1 // PB1, attiny pin 6

uint16_t loop_period = 1000; // in ms

double kp = 4.0;
double ki = 0.4;
double kd = 0.1;

double setpoint = 512;

// ---

double input = 0;
double output = 0;
PID myPID(&input, &output, &setpoint, kp, ki, kd, REVERSE);

SoftwareSerial ser(RX,TX);

uint32_t next_loop_start = 0;

void setup() {

  analogReference(EXTERNAL);
  
  pinMode(INPUT_PIN, INPUT);
  pinMode(OUTPUT_PIN, OUTPUT);
  
  ser.begin(9600); // 9600 8N1

  myPID.SetSampleTime(loop_period);
  myPID.SetMode(AUTOMATIC);

  // print out the CSV header.
  delay(250);
  ser.println("input,output");

  next_loop_start = millis() + loop_period;
}


void loop() {

  if (input != 0) {
    ser.print(input);
    ser.print(",");
    ser.println(output);
  }
  
  // accumulate samples until we only have 50ms of time margin left in this loop
  uint32_t accumulator = 0;
  uint32_t samples = 0;
  while (next_loop_start - millis() > 50) {
    for (uint8_t i = 0; i < UINT8_MAX; i++) {
      accumulator += analogRead(INPUT_PIN);
    }
    samples += UINT8_MAX;
  }
  input = double(accumulator) / double(samples);

  while (myPID.Compute() == false) { continue; }
  analogWrite(OUTPUT_PIN, output);

  while (millis() <= next_loop_start) { delay(1); }
  while (millis() > next_loop_start) { next_loop_start += loop_period; }
}

