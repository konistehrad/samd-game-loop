#ifndef KAHN_FAST_DIGITAL_H
#define KAHN_FAST_DIGITAL_H

#include <Arduino.h>

// Fast read/write code from here: https://forum.arduino.cc/index.php?topic=129868.15
// Broke it up to save a compare when you know what you're writing
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

#endif
