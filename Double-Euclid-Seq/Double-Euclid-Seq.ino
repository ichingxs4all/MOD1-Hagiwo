/*
※Generate a rhythm pattern based on the entered clock

※The trigger length is set to 5ms. The trigger length can be changed at delayMicroseconds(5000);. ( 5ms=5000)

POT1: Set the number of triggers used by the Euclidian pattern of output 1.

POT2: Set the number of triggers to be used by the Euclidian pattern of output 2.

POT3: Divide the Euclidian pattern set in output 2 to D10pin and D11pin with probability. Approximately 50:50 in the middle, always output from D10pin when minimized, always output from D11pin when maximized

F1: Trigger IN

F2: Trigger output of output 1

F3: Trigger output 1 of output 2

F4: Trigger output 2 of output 2

LED: Linked to F1

*/

// 2-output Euclidean sequencer with probabilistic branching like Mutable Instruments Branches

// Now includes 4-step delay on Output 1 using an 8-bit shift register emulation

const int triggerInputPin = 17;

const int buttonPin = 2;

const int ledPin = 3;

const int out1Pin = 9;

const int out2aPin = 10;

const int out2bPin = 11;

const int knob1Pin = A0;

const int knob2Pin = A1;

const int probKnobPin = A2;

const int numSteps = 16;

bool pattern1[numSteps];

bool pattern2[numSteps];

int currentStep = 0;

unsigned long lastClockTime = 0;

bool internalClock = false;

unsigned long lastTapTime = 0;

unsigned long bpmInterval = 0;

bool lastButtonState = HIGH;

bool lastTriggerState = LOW;

unsigned long lastTriggerReceived = 0;

const unsigned long triggerTimeout = 300;

// — 8-bit shift register for delayed output1 —

const int shiftSize = 8;

bool shiftRegister[shiftSize] = {false}; // stores pattern1 history

int shiftIndex = 0;


// — Functions —

void generateEuclideanPattern(bool* pattern, int pulses) {

for (int i = 0; i < numSteps; i++) {

pattern[i] = (i * pulses) % numSteps < pulses;

}

}

void sendTrigger(int pin) {

digitalWrite(pin, HIGH);

delayMicroseconds(5000);

digitalWrite(pin, LOW);

}


int readKnobSteps(int analogPin) {

int raw = analogRead(analogPin);

int step = raw / (1024 / (numSteps + 1));

if (step > numSteps) step = numSteps;

return step;

}


float readProbability() {

return analogRead(probKnobPin) / 1023.0;

}


void setup() {

pinMode(triggerInputPin, INPUT);

pinMode(buttonPin, INPUT_PULLUP);

pinMode(ledPin, OUTPUT);

pinMode(out1Pin, OUTPUT);

pinMode(out2aPin, OUTPUT);

pinMode(out2bPin, OUTPUT);

generateEuclideanPattern(pattern1, 0);

generateEuclideanPattern(pattern2, 0);

randomSeed(analogRead(A3));

lastTriggerState = digitalRead(triggerInputPin);

lastTriggerReceived = millis();

}


bool checkClock() {

bool triggered = false;

bool currentTrigger = digitalRead(triggerInputPin);

unsigned long now = millis();

if (currentTrigger &&! lastTriggerState) {

if ((now - lastTriggerReceived) > triggerTimeout) {

currentStep = 0;

}

lastTriggerReceived = now;

lastClockTime = now;

internalClock = false;

triggered = true;

}

lastTriggerState = currentTrigger;

if (internalClock && bpmInterval > 0 && now - lastClockTime >= bpmInterval) {

lastClockTime = now;

triggered = true;

}

return triggered;

}

void updateTapTempo() {

bool currentButton = digitalRead(buttonPin);

if (! currentButton && lastButtonState) {

unsigned long now = millis();

if (lastTapTime > 0) {

bpmInterval = now - lastTapTime;

internalClock = true;

}

lastTapTime = now;

}

lastButtonState = currentButton;

}

void loop() {

updateTapTempo();

if (checkClock()) {

digitalWrite(ledPin, HIGH);

delayMicroseconds(1000);

digitalWrite(ledPin, LOW);

int steps1 = readKnobSteps(knob1Pin);

int steps2 = readKnobSteps(knob2Pin);

generateEuclideanPattern(pattern1, steps1);

generateEuclideanPattern(pattern2, steps2);

// — Store current pattern1 state into shift register —

shiftRegister[shiftIndex] = pattern1[currentStep];

// — Calculate delayed index (4 steps behind current) —

int delayedIndex = (shiftIndex + shiftSize - 4) % shiftSize;

if (shiftRegister[delayedIndex]) {

sendTrigger(out1Pin); // delayed output

}

// — Normal Output 2 with branching —

if (pattern2[currentStep]) {

float prob = readProbability();

if (random(0, 1000) < prob * 1000.0) {

sendTrigger(out2bPin);

} else {

sendTrigger(out2aPin);

}

}

// — Advance step and shift index —

currentStep = (currentStep + 1) %numSteps;

shiftIndex = (shiftIndex + 1) % shiftSize;

}

}