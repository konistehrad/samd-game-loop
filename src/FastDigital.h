#ifndef KAHN_FAST_DIGITAL_H
#define KAHN_FAST_DIGITAL_H

#include <Arduino.h>

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

int inline analogReadFast(byte ADCpin);

#if defined(__arm__) 
int inline analogReadFast(byte ADCpin)    // inline library functions must be in header
{ ADC->CTRLA.bit.ENABLE = 0;              // disable ADC
  while( ADC->STATUS.bit.SYNCBUSY == 1 ); // wait for synchronization

  int CTRLBoriginal = ADC->CTRLB.reg;
  int AVGCTRLoriginal = ADC->AVGCTRL.reg;
  int SAMPCTRLoriginal = ADC->SAMPCTRL.reg;
  
  ADC->CTRLB.reg &= 0b1111100011111111;          // mask PRESCALER bits
  ADC->CTRLB.reg |= ADC_CTRLB_PRESCALER_DIV64;   // divide Clock by 64
  ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1 |   // take 1 sample 
                     ADC_AVGCTRL_ADJRES(0x00ul); // adjusting result by 0
  ADC->SAMPCTRL.reg = 0x00;                      // sampling Time Length = 0

  ADC->CTRLA.bit.ENABLE = 1;                     // enable ADC
  while(ADC->STATUS.bit.SYNCBUSY == 1);          // wait for synchronization

  int adc = analogRead(ADCpin); 
  
  ADC->CTRLB.reg = CTRLBoriginal;
  ADC->AVGCTRL.reg = AVGCTRLoriginal;
  ADC->SAMPCTRL.reg = SAMPCTRLoriginal;
   
  return adc;
}
#else
int inline analogReadFast(byte ADCpin) 
{ byte ADCSRAoriginal = ADCSRA; 
  ADCSRA = (ADCSRA & B11111000) | 4; 
  int adc = analogRead(ADCpin);  
  ADCSRA = ADCSRAoriginal;
  return adc;
}
#endif

#endif
