#include <Arduino.h>

// KPR-77-MIDI-Interface by Leo Kuroshita
// v 0.1 | Under MIT 

/***********************************************************
 * PIN DEFINITIONS  (adjust to your wiring)
 **********************************************************/

// 12 drum trigger outputs (S-TRIG: active LOW)
const uint8_t trigPins[12] = {
  2,3,4,5,6,7,8,9,10,11,12,13
};

// Accent
const uint8_t accentPin = 14;

// DIN SYNC (48 ppqn)
const uint8_t dinClockPin = 15;   // Clock out (48ppqn pulses)
const uint8_t dinRunPin   = 16;   // RUN line (High=running)
const uint8_t dinStopPin  = 17;   // Optional Stop pulse


/***********************************************************
 * TRIGGER MAP
 **********************************************************/

// Default: GM drum → KPR-77 mapping
uint8_t noteMap[12] = {
  35, 36, 38, 40, 41, 45, 48,
  42, 44, 46, 49, 39
};

uint8_t accentNote = 57;

/***********************************************************
 * SYSEX BUFFER
 **********************************************************/
uint8_t sysexBuf[32];
uint8_t sysexPos = 0;
bool inSysex = false;


/***********************************************************
 * TIMING / DIN SYNC STATE
 **********************************************************/
volatile bool running = false;

volatile uint32_t lastF8 = 0;      // last MIDI clock time
volatile uint32_t halfDt = 10000;  // half interval between F8s
volatile uint32_t midDue = 0;
volatile bool midArmed = false;

const uint8_t TRIG_MS = 2;


/***********************************************************
 * LOW-LEVEL PULSES
 **********************************************************/

void trigPulse(uint8_t pin){
  digitalWrite(pin, LOW);
  delay(TRIG_MS);
  digitalWrite(pin, HIGH);
}

void accentPulse(){
  digitalWrite(accentPin, LOW);
  delay(TRIG_MS);
  digitalWrite(accentPin, HIGH);
}

void sendClockPulse(){
  digitalWrite(dinClockPin, HIGH);
  delayMicroseconds(250);     // 0.25 ms high pulse
  digitalWrite(dinClockPin, LOW);
}


/***********************************************************
 * SYSEX HANDLER
 **********************************************************/
void handleSysex(uint8_t *d, uint8_t len){
  if (len < 3) return;
  if (d[0] != 0x7D || d[1] != 0x77) return;

  switch (d[2]){
    case 0x01:   // assign drum idx -> note
      if (len >= 5){
        uint8_t idx = d[3];
        uint8_t note = d[4];
        if (idx < 12) noteMap[idx] = note;
      }
      break;

    case 0x02:   // assign accent note
      if (len >= 4){
        accentNote = d[3];
      }
      break;

    default:
      break;
  }
}


/***********************************************************
 * MIDI BYTE PROCESSOR
 **********************************************************/
void processMidi(uint8_t b){
  static uint8_t status = 0;
  static uint8_t data1 = 0;

  /*************** SYSEX ***************/
  if (inSysex){
    if (b == 0xF7){
      inSysex = false;
      handleSysex(sysexBuf, sysexPos);
      sysexPos = 0;
      return;
    }
    if (sysexPos < sizeof(sysexBuf))
      sysexBuf[sysexPos++] = b;
    return;
  }

  if (b == 0xF0){
    inSysex = true;
    sysexPos = 0;
    return;
  }


  /*************** REALTIME ***************/
  if (b >= 0xF8){
    if (b == 0xF8){     // MIDI Clock 24ppqn
      uint32_t now = micros();

      if (lastF8 != 0){
        uint32_t dt = now - lastF8;
        if (dt > 2000 && dt < 200000)     // filter noise
          halfDt = dt >> 1;
      }
      lastF8 = now;

      if (running){
        sendClockPulse();            // first pulse
        midDue = now + halfDt;       // schedule second pulse
        midArmed = true;
      }

    } else if (b == 0xFA){  // Start
      running = true;
      digitalWrite(dinRunPin, HIGH);
      lastF8 = 0;
      midArmed = false;

    } else if (b == 0xFC){  // Stop
      running = false;
      digitalWrite(dinRunPin, LOW);
      midArmed = false;
    }

    return;
  }


  /*************** STATUS ***************/
  if (b & 0x80){
    status = b;
    return;
  }


  /*************** CHANNEL MESSAGES ***************/
  if ((status & 0xF0) == 0x90){    // Note On
    uint8_t note = b;
    while (!Serial.available());
    uint8_t vel = Serial.read();

    if (vel > 0){
      // Drum triggers
      for (uint8_t i=0; i<12; i++){
        if (noteMap[i] == note){
          trigPulse(trigPins[i]);
        }
      }

      // Accent
      if (note == accentNote){
        accentPulse();
      }
    }
  }
}


/***********************************************************
 * SETUP
 **********************************************************/
void setup(){

  Serial.begin(31250);   // MIDI baud

  // S-Trig outputs
  for (uint8_t i=0;i<12;i++){
    pinMode(trigPins[i], OUTPUT);
    digitalWrite(trigPins[i], HIGH);   // idle HIGH
  }

  pinMode(accentPin, OUTPUT);
  digitalWrite(accentPin, HIGH);

  // DIN SYNC
  pinMode(dinClockPin, OUTPUT);
  pinMode(dinRunPin,   OUTPUT);
  pinMode(dinStopPin,  OUTPUT);

  digitalWrite(dinClockPin, LOW);
  digitalWrite(dinRunPin,   LOW);
  digitalWrite(dinStopPin,  LOW);
}


/***********************************************************
 * MAIN LOOP
 **********************************************************/
void loop(){

  // MIDI RX
  if (Serial.available()){
    uint8_t b = Serial.read();
    processMidi(b);
  }

  // second pulse for 48ppqn
  if (midArmed && (int32_t)(micros() - midDue) >= 0){
    sendClockPulse();
    midArmed = false;
  }
}
