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
#include <ArduinoLowPower.h>
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
