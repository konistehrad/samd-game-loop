#include "NeoPixelDisplay.h"

bool NeoPixelDisplay::begin(uint16_t brightness) {
  this->brightness = brightness;
  idxB = strip.numPixels() / 2;
  strip.begin();
  strip.show();
  return true;
}

void NeoPixelDisplay::parseCommand(InputCommand command) {
  switch(command.command) {
    case InputCommand::Brightness_Down: decreaseBrightness(command.payload); break;
    case InputCommand::Brightness_Up  : increaseBrightness(command.payload); break;
    case InputCommand::Hue_Down       : decreaseHue(command.payload); break;
    case InputCommand::Hue_Up         : increaseHue(command.payload); break;
    case InputCommand::Rotation_Mod   : 
      filter.reset(); filter.filter(1); break;
  }
}

void NeoPixelDisplay::tick(Time_t dtMs) {
  rotateTimer += dtMs;
  rotateTime = filter.output();
  if(rotateTimer > rotateTime)  {
    filter.filter(1000);
    idxA = (idxA + 1) % strip.numPixels();
    idxB = (idxB + 1) % strip.numPixels();
    dirty = true;
    rotateTimer = 0;
  }

  if(dirty) {
    auto color = /* Adafruit_NeoPixel::gamma32*/
      (Adafruit_NeoPixel::ColorHSV(hueWrap, 255, brightness));
    // strip.clear();
    strip.fill(Adafruit_NeoPixel::ColorHSV(hueWrap + (USHRT_MAX / 4), 255, brightness));
    strip.setPixelColor(idxA, color);
    strip.setPixelColor(idxB, color);
    dirty = true;
  }

  if(dirty) {
    strip.show();
    dirty = false;
  }
}
