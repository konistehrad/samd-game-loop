#include <Arduino.h>
#include <ArduinoLowPower.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SleepyDog.h>
#include <limits.h>
#include <Thread.h>

#define FPS                   (60.0)

#include "FastDigital.h"
#include "GameLoop.h"
#include "Input.h"

#define MOD_PER_CLICK         (10)
#define LED_COUNT             (10u)

#define PIN_BUTTON_LEFT       (4u)
#define PIN_BUTTON_RIGHT      (5u)
#define PIN_SLIDE_SWITCH      (7u)
#define PIN_NEOPIXEL          (8u)
#define PIN_SPEAKER_SHUTDOWN  (11u)
#define PIN_IR_PROX           (10u)
#define PIN_IR_RECEIVER       (26u)

void gameLoop(Time_t dt);

bool dirty = true;
uint16_t brightness = 1;
uint32_t rotateTimer = 0;
uint32_t rotateTime = 100;
uint16_t hueWrap = USHRT_MAX / 4;
uint16_t idxA = 0;
uint16_t idxB = LED_COUNT / 2;
InputParser input;
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

void checkRotate(Time_t dt) {
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
  Serial.begin(115200);

  Serial.println("Begining initialization!");
  pinMode(LED_BUILTIN, OUTPUT);

  // disable speaker on this sketch
  pinMode(PIN_SPEAKER_SHUTDOWN, OUTPUT);
  digitalWrite(PIN_SPEAKER_SHUTDOWN, LOW);

  // but leave the analog output available
  pinMode(A0, OUTPUT);

  if(!input.begin()) {
    Serial.println("Input initialization failed!");
    while(true) delayFlash(1, 1000);
  }

  strip.begin();
  strip.show();

  GameLoop.setup(gameLoop);
}

void loop() {
  // forward the call!
  GameLoop.loop();
}

void gameLoop(Time_t dt) {
  // use LED_BUILTIN as a sort of CPU load indicator
  digitalWriteDirect(LED_BUILTIN, HIGH);

  auto command = input.tick(dt);
  switch(command.command) {
    case InputCommand::Brightness_Down: decreaseBrightness(command.payload); break;
    case InputCommand::Brightness_Up  : increaseBrightness(command.payload); break;
    case InputCommand::Hue_Down       : decreaseHue(command.payload); break;
    case InputCommand::Hue_Up         : increaseHue(command.payload); break;
  }

  checkRotate(dt);
  animTick();
  pushPixels();
  digitalWriteDirect(LED_BUILTIN, LOW);
}


