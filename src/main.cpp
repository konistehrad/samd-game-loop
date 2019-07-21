#include <Arduino.h>
#include <ArduinoLowPower.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SleepyDog.h>
#include <limits.h>
#include <Thread.h>

#include "FastDigital.h"

#define NUMBER                float
#define Time_t                unsigned long

#define FPS                   (60.0)
#define MS_PER_FRAME          ((unsigned long)(1000.0 / FPS))
#define US_PER_FRAME          ((unsigned long)(1000000.0 / FPS))

#define MOD_PER_CLICK         (10)
#define LED_COUNT             (10u)

#define PIN_BUTTON_LEFT       (4u)
#define PIN_BUTTON_RIGHT      (5u)
#define PIN_SLIDE_SWITCH      (7u)
#define PIN_NEOPIXEL          (8u)
#define PIN_SPEAKER_SHUTDOWN  (11u)
#define PIN_IR_PROX           (10u)
#define PIN_IR_RECEIVER       (26u)

#define DELTA(start,end) ((end >= start) ? (end - start) : (ULONG_MAX - start) + end)

volatile bool dirty = true;
volatile uint16_t brightness = 1;

uint32_t rotateTimer = 0;
uint32_t rotateTime = 100;
uint16_t hueWrap = USHRT_MAX / 4;
uint16_t idxA = 0;
uint16_t idxB = LED_COUNT / 2;
Adafruit_NeoPixel strip(LED_COUNT, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

void delayFlash(uint8_t times, uint32_t delayTime) {
  for(uint8_t i = 0; i < times; ++i) {
    digitalWriteDirect(LED_BUILTIN, HIGH); delay(delayTime);
    digitalWriteDirect(LED_BUILTIN, LOW); delay(delayTime);
  }
}

inline void decreaseBrightness(uint8_t amount) {
  dirty = true;
  brightness = amount > brightness ? 0 : brightness - amount;
}

inline void increaseBrightness(uint8_t amount) {
  uint16_t result = amount + brightness;
  brightness = result > 255 ? 255 : result;
  dirty = true;
}

inline void increaseHue(uint16_t amount) {
  dirty = true; hueWrap += amount;
}

inline void decreaseHue(uint16_t amount) {
  dirty = true; hueWrap -= amount;
}

void commonSetup() {
  Serial.begin(9600);

  delayFlash(2, 1000);

  Serial.println("Begining initialization!");
  pinMode(LED_BUILTIN, OUTPUT);

  // Express input modules
  pinMode(PIN_BUTTON_LEFT,  INPUT_PULLDOWN);
  pinMode(PIN_BUTTON_RIGHT, INPUT_PULLDOWN);
  pinMode(PIN_SLIDE_SWITCH, INPUT_PULLUP);

  // disable speaker on this sketch
  pinMode(PIN_SPEAKER_SHUTDOWN, OUTPUT);
  digitalWrite(PIN_SPEAKER_SHUTDOWN, LOW);

  // but leave the analog output available
  pinMode(A0, OUTPUT);

  strip.begin();
  strip.show();
}

bool state = false;
Time_t now = 0;
Time_t lastTime = 0;
Time_t dt;

inline bool HueMode() { return digitalReadDirect(PIN_SLIDE_SWITCH); }
inline bool ButtonLeft() { return digitalReadDirect(PIN_BUTTON_LEFT); }
inline bool ButtonRight() { return digitalReadDirect(PIN_BUTTON_RIGHT); }

void parseInput() {
  if(HueMode()) {
    if(ButtonLeft()) {
      decreaseHue(map(dt, 0, 4000, 0, USHRT_MAX));
    } else if(ButtonRight()) {
      increaseHue(map(dt, 0, 4000, 0, USHRT_MAX));
    }
  } else {
    if(ButtonLeft()) {
      decreaseBrightness(map(dt, 0, 1000, 0, 64) );
    } else if(ButtonRight()) {
      increaseBrightness(map(dt, 0, 1000, 0, 64) );
    }
  }
}

void animTick(bool force = false) {
  if(dirty || force) {
    auto color = /* Adafruit_NeoPixel::gamma32*/
      (Adafruit_NeoPixel::ColorHSV(hueWrap, 255, brightness));
    // strip.clear();
    strip.fill(Adafruit_NeoPixel::ColorHSV(hueWrap + (USHRT_MAX / 4), 255, brightness));
    strip.setPixelColor(idxA, color);
    strip.setPixelColor(idxB, color);
    dirty = true;
  }
}

void checkRotate() {
  rotateTimer += dt;
  if(rotateTimer > 100) 
  {
    idxA = (idxA + 1) % LED_COUNT;
    idxB = (idxB + 1) % LED_COUNT;
    dirty = true;
    rotateTimer = 0;
  }
}

void pushPixels() {
  if(dirty) {
    strip.show();
    dirty = false;
  }
}

void setup() {
  commonSetup();
  lastTime = millis();
  animTick(true);
}

void loop() {
  Time_t startMicros = micros();
  now = millis();
  dt = DELTA(lastTime, now);
  parseInput();
  checkRotate();
  animTick();


  lastTime = now;

  pushPixels();
  digitalWriteDirect(LED_BUILTIN, LOW);
  Time_t waitTime = MS_PER_FRAME - (DELTA(startMicros, micros()) / 1000);
  Watchdog.enable(waitTime, true);
  LowPower.idle();
  Watchdog.disable();
  digitalWriteDirect(LED_BUILTIN, HIGH);
}


