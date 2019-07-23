#ifndef KAHN_NEO_PIXEL_DISPLAY_H
#define KAHN_NEO_PIXEL_DISPLAY_H

#include <Adafruit_NeoPixel.h>
#include <EwmaT.h>

#include "GameLoop.h"
#include "Input.h"

class NeoPixelDisplay {
  public:
    NeoPixelDisplay(Adafruit_NeoPixel& s) : strip(s) {}
    bool begin(uint16_t brightness);
    void parseCommand(InputCommand command);
    void tick(Time_t dtMs);

    void decreaseBrightness(uint8_t amount) {
      dirty = true; brightness = amount >= brightness ? 1 : brightness - amount;
    }

    void increaseBrightness(uint8_t amount) {
      uint16_t result = amount + brightness;
      brightness = result > 255 ? 255 : result;
      dirty = true;
    }

    void increaseHue(uint16_t amount) { dirty = true; hueWrap += amount; }
    void decreaseHue(uint16_t amount) { dirty = true; hueWrap -= amount; }
  protected:
    bool dirty = true;
    Adafruit_NeoPixel& strip;
    uint16_t brightness = 0;
    uint32_t rotateTimer = 0;
    uint32_t rotateTime = 100;
    EwmaT<uint32_t> filter = EwmaT<uint32_t>(25, 1000);
    uint16_t hueWrap = USHRT_MAX / 4;
    uint16_t idxA = 0;
    uint16_t idxB = 0;

};

#endif
