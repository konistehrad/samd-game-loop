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

#include "Input.h"

volatile uint8_t tapped;
void wakeForAccel() {
  tapped = true;
}

bool InputParser::begin() {
  rotationTime = 0;

  // Express input modules
  pinMode(PIN_BUTTON_LEFT,  INPUT_PULLDOWN);
  pinMode(PIN_BUTTON_RIGHT, INPUT_PULLDOWN);
  pinMode(PIN_SLIDE_SWITCH, INPUT_PULLUP);

  lis = Adafruit_LIS3DH(&CPLAY_LIS3DH_WIRE);

  if(!lis.begin(CPLAY_LIS3DH_ADDRESS)) {
    Serial.println("ACCEL INIT FAILED!");
    return false;
  } else {

    // have a procedure called when a tap is detected
    // use LowPower routines to guarantee wake from sleep/idle
    LowPower.attachInterruptWakeup(CPLAY_LIS3DH_INTERRUPT, wakeForAccel, FALLING);

    lis.setRange(LIS3DH_RANGE_2_G);
    lis.setClick(1, CLICKTHRESHHOLD, 10, 20, 255); 
  }
  return true;
}

InputCommand InputParser::tick(uint32_t dtMs) {
  // acceleration input takes prescidence
  auto result = parseAcceleration(dtMs);
  if(!result.isEmpty()) return result;

  if(hueMode()) {
    if(buttonLeft()) {
      return InputCommand(InputCommand::Hue_Down, map(dtMs, 0, 4000, 0, USHRT_MAX));
    } else if(buttonRight()) {
      return InputCommand(InputCommand::Hue_Up, map(dtMs, 0, 4000, 0, USHRT_MAX));
    }
  } else {
    if(buttonLeft()) {
      return InputCommand(InputCommand::Brightness_Down, map(dtMs, 0, 1000, 0, 64));
    } else if(buttonRight()) {
      return InputCommand(InputCommand::Brightness_Up, map(dtMs, 0, 1000, 0, 64));
    }
  }

  return InputCommand();
}

InputCommand InputParser::parseAcceleration(uint32_t dtMs) {
  if(tapped) {
    tapped = false;
    return InputCommand(InputCommand::Rotation_Mod);
  }
  return InputCommand();
}
