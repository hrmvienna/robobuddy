/*
    HEADER TODO
*/

#ifndef helper_h
#define helper_h

#include "Arduino.h"

inline void dwh(int pin) { //digital write high
  digitalWrite(pin, HIGH);
}
inline void dwl(int pin) { //digital write low
  digitalWrite(pin, LOW);
}
inline int dr(int pin) { //digital read
  return digitalRead(pin);
}
inline void aw(int pin, int value) { //analog write
  analogWrite(pin, value);
}
inline int  ar(int pin) { //analog read
  return analogRead(pin);
}
inline void pmo(int pin) { //set pinmode output
  pinMode(pin, OUTPUT);
}
inline void pmi(int pin) { //set pinmode input
  pinMode(pin, INPUT);
}

/* Function template *//*

/* TODO * /
void NAME(int param) {

}
*/
#endif
