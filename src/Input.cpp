
#include "Input.h"

volatile uint8_t tapped;
void wakeForAccel() {
  Serial.println("Got wakeup message!");
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
