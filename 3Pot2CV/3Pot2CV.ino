/*
HAGIWO MOD1 Pot 2 CV
3ch output Pot 2 CV

--Pin assign---
POT1  A0  
POT2  A1  
POT3  A2  
F1    A3  N/A
F2    A4  CV1 output
F3    A5  CV2 output
F4    D11 CV3 output
BUTTON    N/A
LED       CV1 output
EEPROM    N/A

*/

int outputVal1,outputVal2,outputVal3,cvIn;

// ---------------- Function Prototypes ----------------
void configurePWM();                       // Set up Timer1 and Timer2 for ~62.5kHz PWM
void readPots();

// ------------------------------------------------------
void setup() {
  // Configure I/O pins
  pinMode(9, OUTPUT);    // CV1 (OCR1A)
  pinMode(10, OUTPUT);   // CV2 (OCR1B)
  pinMode(11, OUTPUT);   // CV3 (OCR2A)
  pinMode(3, OUTPUT);    // LED indicator (OCR2B)
  pinMode(4, INPUT_PULLUP); // Push button (pull-up); press => LOW

  // Configure Timer1 & Timer2 to ~62.5kHz PWM
  configurePWM();

}

// ------------------------------------------------------
void loop() {
    readCV_Input();
    readPots();
    readButton();

    // Set duty cycles directly via OCR registers
    // CV1 -> OCR1A (Pin 9)
    // CV2 -> OCR1B (Pin 10)
    // CV3 -> OCR2A (Pin 11)
    // LED indicator (same as CV1) -> OCR2B (Pin 3)

    OCR1A = outputVal1;  
    OCR1B = outputVal2;  
    OCR2A = outputVal3;  
    OCR2B = outputVal1;  // LED shows LFO1's output
}

// ------------------------------------------------------
// Configure Timer1 (16-bit) and Timer2 (8-bit) for ~62.5kHz PWM
void configurePWM() {
  // ---- Timer1 setup (Pins 9=OCR1A, 10=OCR1B) ----
  // Fast PWM 8-bit mode: WGM10=1, WGM11=0, WGM12=1, WGM13=0
  // Non-inverting for OCR1A, OCR1B: COM1A1=1, COM1B1=1
  // No prescaler: CS10=1
  TCCR1A = 0; 
  TCCR1B = 0;
  TCCR1A |= (1 << WGM10) | (1 << COM1A1) | (1 << COM1B1);  
  TCCR1B |= (1 << WGM12) | (1 << CS10);                   

  // ---- Timer2 setup (Pins 3=OCR2B, 11=OCR2A) ----
  // Fast PWM mode (0xFF): WGM20=1, WGM21=1, WGM22=0
  // Non-inverting for OCR2A, OCR2B: COM2A1=1, COM2B1=1
  // No prescaler: CS20=1
  TCCR2A = 0; 
  TCCR2B = 0; 
  TCCR2A |= (1 << WGM20) | (1 << WGM21) | (1 << COM2A1) | (1 << COM2B1); 
  TCCR2B |= (1 << CS20); 
}

void readPots(){
outputVal1 = map(analogRead(A0)+cvIn,0, 1023, 0,255);
outputVal2 = map(analogRead(A1)+cvIn,0, 1023, 0,255);
outputVal3 = map(analogRead(A2)+cvIn,0, 1023, 0,255);
}

void readCV_Input(){
  cvIn = analogRead(A3);
}

void readButton(){
  if(digitalRead(4)==LOW) {
  outputVal1=255;
  outputVal2=255;
  outputVal3=255;
  }
}