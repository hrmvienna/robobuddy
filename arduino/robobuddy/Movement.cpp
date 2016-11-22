/*
    HEADER TODO
*/

#include "Arduino.h"
#include "Movement.h"
#include "helper.h"

// constructor
Movement::Movement() {
  // setup pins
  pmo(PWMA);
  pmo(AIN1);
  pmo(AIN2);
  pmo(PWMB);
  pmo(BIN1);
  pmo(BIN2);
  pmo(STBY);
  dwh(STBY); //todo init value
}
