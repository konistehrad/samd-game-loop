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

#ifndef KAHN_INPUT_H
#define KAHN_INPUT_H

#define PIN_BUTTON_LEFT       (4u)
#define PIN_BUTTON_RIGHT      (5u)
#define PIN_SLIDE_SWITCH      (7u)

#define CPLAY_LIS3DH_INTERRUPT (27)
#define CPLAY_LIS3DH_CS        (-1)
#define CPLAY_LIS3DH_ADDRESS   (0x19)
#define CPLAY_LIS3DH_WIRE      (Wire1)

#define CLICKTHRESHHOLD 120

#include <Arduino.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#include "FastDigital.h"

struct IRButton { enum IRButton_t : uint8_t {
  NONE,
  DPad_UP,
  DPad_DOWN,
  DPad_LEFT,
  DPad_RIGHT,
  DPad_OK,
  Vol_UP,
  Vol_DOWN,

}; };

struct InputCommand { 
  const static InputCommand NO_COMMAND;

  // this used to be 8-bit, but with the 
  // necessity of a 32-bit payload let's just
  // keep everyone nice and 4-byte aligned
  enum Command_t : uint32_t {
    NONE,
    Brightness_Down, Brightness_Up,
    Hue_Down, Hue_Up,
    Rotation_Mod,
  }; 

  Command_t command = NONE;
  uint32_t payload = 0;

  InputCommand() : command(NONE), payload(0) {}
  InputCommand(Command_t c) : command(c), payload(0) {}
  InputCommand(Command_t c, uint32_t p) : command(c), payload(p) {}
  bool isEmpty() { return command == NONE; }
};

class InputParser {
  public: 
    bool begin();
    InputCommand tick(uint32_t dtMs);
    uint32_t getRotationTime() const { return rotationTime; };
  protected:
    bool hueMode() { return digitalReadDirect(PIN_SLIDE_SWITCH); }
    bool buttonLeft() { return digitalReadDirect(PIN_BUTTON_LEFT); }
    bool buttonRight() { return digitalReadDirect(PIN_BUTTON_RIGHT); }
    InputCommand parseAcceleration(uint32_t dtMs);

    Adafruit_LIS3DH lis;
    uint32_t rotationTime;
};

#endif
