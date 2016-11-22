/*
    HEADER TODO
*/

#ifndef Leds_h
#define Leds_h

#include "Arduino.h"

// pin assignment - status leds
#define LED_IDLE A3
#define LED_FINISHED A2
#define LED_ERROR A1
#define LED_BUSY A0

// prototypes - helper functions
inline void ledOn(int pin);
inline void ledOff(int pin);

class Leds
{
    public:
        Leds();
        inline void allOff();
        inline void busy();
        inline void finished();
        inline void error();
        inline void idle();
    private:
        int state;
};

/* turn all leds off */
inline void Leds::allOff() {
  ledOff(LED_BUSY);
  ledOff(LED_FINISHED);
  ledOff(LED_ERROR);
  ledOff(LED_IDLE);
}

/* turn busy led on, all other off */
inline void Leds::busy() {
  ledOn(LED_BUSY);
  ledOff(LED_FINISHED);
  ledOff(LED_ERROR);
  ledOff(LED_IDLE);
}

/* turn finished led on, all other off */
inline void Leds::finished() {
  ledOff(LED_BUSY);
  ledOn(LED_FINISHED);
  ledOff(LED_ERROR);
  ledOff(LED_IDLE);
}

/* turn error led on, all other off */
inline void Leds::error() {
  ledOff(LED_BUSY);
  ledOff(LED_FINISHED);
  ledOn(LED_ERROR);
  ledOff(LED_IDLE);
}

/* turn idle led on */
inline void Leds::idle() {
  //ledOff(LED_BUSY);
  //ledOff(LED_FINISHED);
  //ledOff(LED_ERROR);
  ledOn(LED_IDLE);
}


/*--------- helper ---------*/
inline void ledOn(int pin) {
  digitalWrite(pin, HIGH);
}

inline void ledOff(int pin) {
  digitalWrite(pin, LOW);
}

#endif
