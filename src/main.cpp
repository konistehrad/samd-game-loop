#include <Arduino.h>
#include <ArduinoLowPower.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SleepyDog.h>
#include <limits.h>
#include <IRLibGlobals.h>
#include <IRLibRecvPCI.h>
#include <IRLibDecodeBase.h>
#include <IRLib_P02_Sony.h>
#include <Thread.h>

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

volatile uint16_t brightness = 1;
uint16_t hueWrap = 0;
Adafruit_NeoPixel strip(LED_COUNT, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
IRrecvPCI irReceiver(PIN_IR_RECEIVER);
IRdecodeSony irDecoder;

inline void decreaseBrightness(uint8_t amount) {
  brightness = amount > brightness ? 0 : brightness - amount;
  // strip.setBrightness(brightness);
}

inline void increaseBrightness(uint8_t amount) {
    uint16_t result = amount + brightness;
    brightness = result > 255 ? 255 : result;
    // strip.setBrightness(brightness);
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
  // strip.fill(0xff0000);
  // strip.setBrightness(brightness);
  strip.show();

  irReceiver.enableIRIn(); // Start the receiver
}

void tryParseIR() {
  if(irReceiver.getResults()) {
    if(irDecoder.decode() && irDecoder.protocolNum == SONY) {
      switch(irDecoder.value) {
        case 0x240C: increaseBrightness(1); break; // vol up
        case 0x640C: decreaseBrightness(1); break; // vol down
        case 0x6F0D: hueWrap += 500; break; // arrow right
        case 0x2F0D: hueWrap -= 500; break; // arrow left
        case 0x0F0D: increaseBrightness(20); break; // arrow up
        case 0x4F0D: decreaseBrightness(20); break; // arrow down
        case 0x180C: break; // enter
        default:     irDecoder.dumpResults(false);
      }
    }
    irReceiver.enableIRIn();
  }
}

bool frameDropped = false;
bool state = false;
unsigned long now = 0;
unsigned long lastTime = 0;
float dt;

void ledBusyFlash() {
  state = !state;
  strip.setPixelColor(LED_COUNT - 1, state ? 20 : 0, 0, 0);
}

void animTick() {
  hueWrap += ((USHRT_MAX / 4) * dt);
  auto color = strip.ColorHSV(hueWrap, 255, brightness);
  strip.setPixelColor(0, color);
  strip.show();
}

Thread ledFlashThread = Thread(ledBusyFlash, 200);
Thread animationThread = Thread(animTick, MS_PER_FRAME);
Thread irThread = Thread(tryParseIR, 0);

void leftButtonInterrupt() {
  if(digitalReadDirect(PIN_BUTTON_LEFT)) {
    decreaseBrightness(10);
  }
}

void rightButtonInterrupt() {
  if(digitalReadDirect(PIN_BUTTON_RIGHT)) {
    increaseBrightness(10);
  }
}

void setup() {
  commonSetup();
  LowPower.attachInterruptWakeup(PIN_BUTTON_LEFT, leftButtonInterrupt, CHANGE);
  LowPower.attachInterruptWakeup(PIN_BUTTON_RIGHT, rightButtonInterrupt, CHANGE);
  lastTime = millis();
}

inline Time_t getDeltaTime(Time_t start, Time_t end) {
  return (end >= start) ? 
    (end - start) :            // standard delta
    (ULONG_MAX - start) + end; // wraparound case
}

void loop() {
  bool irReading = recvGlobal.currentState == STATE_RUNNING || recvGlobal.currentState == STATE_FINISHED;
  now = millis();
  Time_t deltaMs = getDeltaTime(lastTime, now);
  dt = deltaMs / 1000.0;

  Time_t startUs = micros();
  if(ledFlashThread.shouldRun(now)) ledFlashThread.run();
  if(animationThread.shouldRun(now)) animationThread.run();
  if(irThread.shouldRun(now)) irThread.run();
  Time_t endUs = micros();

  lastTime = now;
  Time_t frameUs = getDeltaTime(startUs, endUs);
  frameDropped = (frameUs > US_PER_FRAME);
  auto frameMs = frameDropped ? MS_PER_FRAME : frameUs / 1000;
  if(!irReading && frameMs < MS_PER_FRAME) {
    auto sleepTime = MS_PER_FRAME - frameMs;
    Watchdog.enable(sleepTime, true);
    LowPower.idle();
    Watchdog.disable();
    
    state = !state;
    digitalWriteDirect(LED_BUILTIN, state);
  } 
}
