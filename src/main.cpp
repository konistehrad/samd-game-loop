#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <IRLibAll.h>
#include <Reactduino.h>

#define FPS 30

#define MOD_PER_CLICK         10
#define LED_COUNT             (10u)

#define PIN_IR_PROX           (10u)
#define PIN_BUTTON_LEFT       (4u)
#define PIN_BUTTON_RIGHT      (5u)
#define PIN_SLIDE_SWITCH      (7u)
#define PIN_NEOPIXEL          (8u)
#define PIN_SPEAKER_SHUTDOWN  (11u)
#define PIN_IR_RECEIVER       (26u)

inline void digitalWriteHigh(int PIN) {
  PORT->Group[g_APinDescription[PIN].ulPort].OUTSET.reg = (1ul << g_APinDescription[PIN].ulPin);
}

inline void digitalWriteLow(int PIN) {
  PORT->Group[g_APinDescription[PIN].ulPort].OUTCLR.reg = (1ul << g_APinDescription[PIN].ulPin);
}

inline int digitalReadDirect(int PIN) {
  return !!(PORT->Group[g_APinDescription[PIN].ulPort].IN.reg & (1ul << g_APinDescription[PIN].ulPin));
}

inline void digitalWriteDirect(int PIN, bool val) {
  if(val)  digitalWriteHigh(PIN);
  else     digitalWriteLow(PIN);
}

uint32_t currentColor = 0x33;
uint32_t ledColors[LED_COUNT] = {0};
uint16_t currentIdx = 0;
uint16_t brightness = 1;
uint16_t hueWrap = 0;
Adafruit_NeoPixel strip(LED_COUNT, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
IRrecvPCI irReceiver(PIN_IR_RECEIVER);
IRdecodeSony irDecoder; 

void decreaseBrightness(uint8_t amount) {
  brightness = amount > brightness ? 0 : brightness - amount;
  strip.setBrightness(brightness);
}

void increaseBrightness(uint8_t amount) {
    uint16_t result = amount + brightness;
    brightness = result > 255 ? 255 : result;
    strip.setBrightness(brightness);
}

// bool shadowState;
//------------------------------------------------------------------------------
Reactduino app([] () {
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
  strip.setBrightness(brightness);
  strip.fill(0x330000);
  strip.show();

  irReceiver.enableIRIn(); // Start the receiver

  app.onPinRisingNoInt(PIN_BUTTON_LEFT , [] () { decreaseBrightness(10); });
  app.onPinRisingNoInt(PIN_BUTTON_RIGHT, [] () { increaseBrightness(10); });
  app.onTick([] () {
    int x = digitalReadDirect(PIN_IR_RECEIVER);
    digitalWriteDirect(LED_BUILTIN, !x);

    if(irReceiver.getResults()) {
      if(irDecoder.decode() && irDecoder.protocolNum == SONY) {
        switch(irDecoder.value) {
          case 0x240C: increaseBrightness(1); break;
          case 0x640C: decreaseBrightness(1); break;
          case 0x6F0D: hueWrap += 500; break; // right  decreaseBrightness(); break;
          case 0x2F0D: hueWrap -= 500; break; // left
          case 0x0F0D: increaseBrightness(20); break; // up
          case 0x4F0D: decreaseBrightness(20); break; // down
          case 0x180C: break; // enter

          default:     irDecoder.dumpResults(false);
        }
      }
      irReceiver.enableIRIn();  
    }
  });

  app.repeat((1000 / FPS), [] () {
    strip.fill(strip.ColorHSV(hueWrap, 255, 255));
    strip.show();

    /*
    uint16_t i = currentIdx % LED_COUNT;
    ledColors[i] ^= currentColor;
    strip.setPixelColor(i, ledColors[i]);
    strip.show();
    currentIdx += 1;
    if(currentIdx >= (LED_COUNT * 2)) {
      currentColor = currentColor << 8;
      if(currentColor == 0 || currentColor == 0x33000000) currentColor = 0x33;
      currentIdx = 0;
    }
    */
  });
});
