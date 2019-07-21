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

#ifndef KAHN_GAMELOOP_H
#define KAHN_GAMELOOP_H

#include <Arduino.h>
#include <ArduinoLowPower.h>
#include <Adafruit_SleepyDog.h>

#ifndef FPS
  #define FPS (60.0)
#endif

#ifndef Time_t
  #define Time_t unsigned long
#endif

#ifndef Number_t
  #define Number_t float
#endif

#define MS_PER_FRAME          ((unsigned long)(1000.0 / FPS))
#define US_PER_FRAME          ((unsigned long)(1000000.0 / FPS))

#define KAHN_DELTA(start,end) ((end >= start) ? (end - start) : (ULONG_MAX - start) + end)

typedef void (*gameLoopFuncPtr)(Time_t);

class GameLoop_t {
  public:
    void setup(gameLoopFuncPtr c) { 
      callback = c; 
      lastTime = millis(); 
    }

    void loop() {
      Time_t startMicros = micros();
      now = millis();
      dt = KAHN_DELTA(lastTime, now);
      
      callback(dt);

      lastTime = now;
      Time_t waitTime = MS_PER_FRAME - (KAHN_DELTA(startMicros, micros()) / 1000);
      // we use both watchdog and LowPower here, which seems like a lot but hear me out:
      // Watchdog allows us to easily set sleep timers with ms resolution
      //     LowPower only support second-resolution via RTC
      // However, LowPower has the not-so-aggressive `idle` (sleep is too much for us)
      // and also allows us to assign interrupts to inputs if we so desire. It's a pain,
      // but it really is the most flexible solution
      Watchdog.enable(waitTime, true);
      LowPower.idle();
      Watchdog.disable();
    }

  protected:
    gameLoopFuncPtr callback;
    Time_t now;
    Time_t dt;
    Time_t lastTime;
};

GameLoop_t GameLoop;

#endif
