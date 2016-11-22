/*
    HEADER TODO
*/

#ifndef LineSensors_h
#define LineSensors_h

#include "Arduino.h"

// pin assignment - light sensors
#define LS_LEFT  4
#define LS_RIGHT 6
#define LS_ROAD_1 13
#define LS_ROAD_2 2
#define LS_STATION 10
#define READ_DELAY 10

// prototypes - helper functions
inline int readSensor(int sensor_pin);

class LineSensors
{
  public:
    LineSensors();
    inline int getLeft();
    inline int getRight();
    inline int getRoad1();
    inline int getRoad2();
    inline int getStation();
};

/* read left line sensor */
inline int LineSensors::getLeft() {
  return readSensor(LS_LEFT);
}

/* read right line sensor */
inline int LineSensors::getRight() {
  return readSensor(LS_RIGHT);
}

/* read road line sensor 1 */
inline int LineSensors::getRoad1() {
  return readSensor(LS_ROAD_1);
}

/* read road line sensor 2 */
inline int LineSensors::getRoad2() {
  return readSensor(LS_ROAD_2);
}

/* read station line sensor */
inline int LineSensors::getStation() {
  return readSensor(LS_STATION);
}


/*--------- helper ---------*/
/* read line sensor of the given pin */
inline int readSensor(int sensor_pin) {
  pinMode(sensor_pin, OUTPUT);
  digitalWrite(sensor_pin, HIGH);
  delayMicroseconds(READ_DELAY);

  pinMode(sensor_pin,INPUT);
  long time = micros();
  while(digitalRead(sensor_pin) == HIGH && micros() - time < 3000) {}

  int diff = micros() - time;
  return diff;
}
#endif
