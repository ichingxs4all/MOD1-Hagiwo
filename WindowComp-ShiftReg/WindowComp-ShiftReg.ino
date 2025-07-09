/*
The input voltage is measured in the range of 0V-5V and a 5V gate signal is output.

※When you enter a trigger, the output passes through the shift register and the gate signal that matches the trigger timing is output.

POT1: SHIFT (DETERM THE CENTER OF THE WINDOW)

POT2: SIZE (control the width of the window)

POT3: When the trigger is entered in F2, F3 and F4 are output by shift resist. The output timing can be selected from 1-bit to 8 bits with the knob

F1: CV IN

F2: Trigger IN (can be used as a normal comparator if there is no input)

F3: Comparate output

F4: NOT output of F3

LED: Linked to F3

*/


// Pin definitions

const int shiftKnob = A0;  // SHIFT control knob
const int sizeKnob = A1;  // SIZE control knob
const int shiftRegKnob = A2;  // Shift register control knob
const int inputVoltage = A3;  // Voltage input pin (D17 on Nano)
const int triggerInput = 9;  // Trigger input pin
const int gateOutput = 10;  // Gate output pin
const int notOutput = 11;  // NOT gate output pin
const int ledIndicator = 3;  // LED indicator output pin (same PWM as D10)

// Variables

float shiftValue = 0;
float sizeValue = 0;
int shiftRegValue = 0;
bool shiftRegisterEnabled = false;
byte shiftRegisterState = 0;

bool lastTriggerState = LOW;

unsigned long lastTriggerTime = 0;

const unsigned long triggerTimeout = 500;  // Timeout in ms to return to normal mode

float smoothShift = 0;

float smoothSize = 0;

const float alpha = 0.05;  // Smoothing factor (0: no smoothing, 1: instant)

void setup() {

  pinMode(triggerInput, INPUT);

  pinMode(gateOutput, OUTPUT);

  pinMode(notOutput, OUTPUT);

  pinMode(ledIndicator, OUTPUT);
}

void loop() {

  // Read and smooth control knobs

  int rawShift = analogRead(shiftKnob);

  int rawSize = analogRead(sizeKnob);

  // Apply exponential smoothing

  smoothShift = alpha * rawShift + (1 – alpha) * smoothShift;

  smoothSize = alpha * rawSize + (1 – alpha) * smoothSize;

  // Map smoothed values from 0–1023 to 0–5000 mV for 0–5V input range

  shiftValue = map(smoothShift, 0, 1023, 0, 5000);

  sizeValue = map(smoothSize, 0, 1023, 0, 5000);  // Max window width 5V

  shiftRegValue = constrain(analogRead(shiftRegKnob) / 128, 1, 8);  // 1-8 bit range

  // Read input voltage assuming 0V to 5V mapped to 0–1023

  int inputVal = analogRead(inputVoltage);  // 0-1023

  int voltage = map(inputVal, 0, 1023, 0, 5000);  // 0 to 5000 mV (0V to 5V)

  // Calculate window in mV

  int lowerBound = shiftValue – (sizeValue / 2);

  int upperBound = shiftValue + (sizeValue / 2);

  // Determine the comparator result

  bool gateState = (voltage >= lowerBound && voltage <= upperBound);

  // Read trigger

  bool triggerState = digitalRead(triggerInput);

  // On rising edge, sample gate state and shift

  if (triggerState == HIGH && lastTriggerState == LOW) {

    shiftRegisterEnabled = true;

    lastTriggerTime = millis();

    bool sampledGateState = gateState;

    shiftRegisterState = (shiftRegisterState << 1) | sampledGateState;

    shiftRegisterState &= (1 << shiftRegValue) – 1;
  }

  lastTriggerState = triggerState;

  // Disable shift register mode after timeout

  if (millis() – lastTriggerTime > triggerTimeout) {

    shiftRegisterEnabled = false;
  }

  // Output gate or shifted gate

  bool finalGateState;

  if (shiftRegisterEnabled) {

    finalGateState = (shiftRegisterState >> (shiftRegValue – 1)) & 1;

  } else {

    finalGateState = gateState;
  }

  digitalWrite(gateOutput, finalGateState);

  digitalWrite(notOutput, !finalGateState)

    // LED indicator follows gate output (PWM)

    analogWrite(ledIndicator, finalGateState ? 255 : 0);
}