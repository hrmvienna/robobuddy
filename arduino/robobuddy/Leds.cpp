/*
    HEADER TODO
*/

#include "Arduino.h"
#include "Leds.h"
#include "helper.h"

// constructor
Leds::Leds() {
  // setup pins
  pmo(LED_IDLE);
  pmo(LED_FINISHED);
  pmo(LED_ERROR);
  pmo(LED_BUSY);
}
