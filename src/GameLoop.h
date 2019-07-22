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
#include <Adafruit_SleepyDog.h>

#ifndef FPS
  #define FPS (60.0)
#endif

#ifndef Time_t
  #define Time_t unsigned long
  #define Time_t_MAX   (ULONG_MAX)
#endif

#ifndef Number_t
  #define Number_t float
#endif

#define MS_PER_FRAME          ((Time_t)(1000.0 / FPS))
#define US_PER_FRAME          ((Time_t)(1000000.0 / FPS))
#define US_WAIT_THRESHOLD     ((Time_t)3000)     // at least 3ms is required to go into idle mode

#define KAHN_DELTA(start,end) ((end >= start) ? (end - start) : (Time_t_MAX - start) + end)

typedef void (*gameLoopFuncPtr)(Time_t);

class GameLoop {
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
      // Do we have more than 3ms to wait?
      Time_t deltaMicros = (KAHN_DELTA(startMicros, micros()));
      if(deltaMicros < (US_PER_FRAME - US_WAIT_THRESHOLD)) 
      {
        // TODO: can we avoid the divide here? Might be slow on M0.
        // XXX: would deltaMicros >> 10 work? Is it close enough?
        Time_t waitMs = (US_PER_FRAME - deltaMicros) >> 10;
        idle(waitMs);
      }
    }

  protected:
    gameLoopFuncPtr callback;
    Time_t now;
    Time_t dt;
    Time_t lastTime;
    void idle(uint32_t waitTimeMs) {
      if(waitTimeMs == 0) return;

      Watchdog.enable(waitTimeMs, true);
      SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
      PM->SLEEP.reg = 2;
      __DSB();
      __WFI();
      Watchdog.disable();
    }
};

#endif
