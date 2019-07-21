#include <Arduino.h>
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

volatile uint16_t brightness = 1;
uint16_t hueWrap = 0;
Adafruit_NeoPixel strip(LED_COUNT, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

inline void decreaseBrightness(uint8_t amount) {
  brightness = amount > brightness ? 0 : brightness - amount;
}

inline void increaseBrightness(uint8_t amount) {
  uint16_t result = amount + brightness;
  brightness = result > 255 ? 255 : result;
}

void commonSetup() {
  Serial.begin(115200);
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

  // irReceiver.enableIRIn(); // Start the receiver
}

bool state = false;
Time_t now = 0;
Time_t lastTime = 0;
Time_t dt;

void animTick() {
  if(digitalReadDirect(PIN_SLIDE_SWITCH)) {
    if(digitalReadDirect(PIN_BUTTON_LEFT)) {
      hueWrap -= map(dt, 0, 4000, 0, USHRT_MAX);
    } else if(digitalReadDirect(PIN_BUTTON_RIGHT)) {
      hueWrap += map(dt, 0, 4000, 0, USHRT_MAX);
    }
  } else {
    if(digitalReadDirect(PIN_BUTTON_LEFT)) {
      decreaseBrightness( map(dt, 0, 1000, 0, 255) );
    } else if(digitalReadDirect(PIN_BUTTON_RIGHT)) {
      increaseBrightness( map(dt, 0, 1000, 0, 255) );
    }
  }

  auto color = strip.ColorHSV(hueWrap, 255, brightness);
  strip.setPixelColor(0, color);
  strip.show();
}

void setup() {
  commonSetup();
  lastTime = millis();
}

void loop() {
  now = millis();
  dt = DELTA(lastTime, now);
  animTick();
  lastTime = now;

  digitalWriteDirect(LED_BUILTIN, digitalReadDirect(PIN_SLIDE_SWITCH));
  delay(MS_PER_FRAME / 2);
  // Watchdog.sleep(MS_PER_FRAME / 2);
}
