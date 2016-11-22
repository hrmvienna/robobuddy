/*
    HEADER TODO
*/

#include "Arduino.h"
#include "LineSensors.h"

// prototypes
inline int readSensor(int sensor_pin);

// constructor
LineSensors::LineSensors() {
  // setup pins
  pinMode(LS_LEFT, INPUT);
  pinMode(LS_RIGHT, INPUT);
  pinMode(LS_ROAD_1, INPUT);
  pinMode(LS_ROAD_2, INPUT);
  pinMode(LS_STATION, INPUT);
}
