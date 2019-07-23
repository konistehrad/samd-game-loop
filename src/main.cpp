/*
Copyright 2019 Conrad Kreyling

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
documentation files (the "Software"), to deal in the Software without restriction, including without limitation 
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR 
THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <Arduino.h>
#include <Adafruit_NeoPixel_ZeroDMA.h>

#define FPS                   (60.0)

#include "FastDigital.h"
#include "GameLoop.h"
#include "Input.h"
#include "NeoPixelDisplay.h"

#define LED_COUNT             (10u)

#define PIN_BUTTON_LEFT       (4u)
#define PIN_BUTTON_RIGHT      (5u)
#define PIN_SLIDE_SWITCH      (7u)
#define PIN_NEOPIXEL          (8u)
#define PIN_SPEAKER_SHUTDOWN  (11u)
#define PIN_IR_PROX           (10u)
#define PIN_IR_RECEIVER       (26u)

void gameLoop(Time_t dt);

InputParser input;
Adafruit_NeoPixel_ZeroDMA strip(LED_COUNT, PIN_NEOPIXEL, NEO_GRB);
NeoPixelDisplay display(strip);
GameLoop looper;

void delayFlash(uint8_t times, uint32_t delayTime) {
  for(uint8_t i = 0; i < times; ++i) {
    digitalWriteDirect(LED_BUILTIN, HIGH); delay(delayTime);
    digitalWriteDirect(LED_BUILTIN, LOW); delay(delayTime);
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

  if(!display.begin(1)) {
    Serial.println("Display initialization failed!");
    while(true) delayFlash(1, 300);
  }

  looper.setup(gameLoop);
}

void loop() {
  // forward the call!
  looper.loop();
}

void gameLoop(Time_t dt) {
  // use LED_BUILTIN as a sort of CPU load indicator
  digitalWriteDirect(LED_BUILTIN, HIGH);

  auto command = input.tick(dt);
  display.parseCommand(command);
  display.tick(dt);
  
  digitalWriteDirect(LED_BUILTIN, LOW);
}
