/*
    HEADER TODO
*/

#ifndef Movement_h
#define Movement_h

#include "Arduino.h"
#include "helper.h"

// pin assignment - motor driver
#define PWMA 5
#define PWMB 3
#define AIN1 9
#define AIN2 11
#define BIN1 8
#define BIN2 7
#define STBY 12

// prototypes - helper functions
inline void setA(int ain1, int ain2);
inline void setB(int bin1, int bin2);
inline void setPins(int pin1, int val1, int pin2, int val2);

class Movement
{
    public:
        Movement();
        inline void standby(int state);
        inline void stop();

        inline void forward(int speed);
        inline void forwardleft(int speed);
        inline void forwardright(int speed);

        inline void backward(int speed);
        inline void backwardleft(int speed);
        inline void backwardright(int speed);

        inline void spinleft(int speed);
        inline void spinright(int speed);
    private:
        int state;
};

/* TODO */
inline void Movement::standby(int state) {
  digitalWrite(STBY, state);
}

/* stop both motors */
inline void Movement::stop() {
  dwl(STBY);
  setA(LOW, LOW);
  analogWrite(PWMA, HIGH);
  setB(LOW, LOW);
  analogWrite(PWMB, HIGH);
}

/* turn both motors forward */
inline void Movement::forward(int speed) {
  setA(LOW, HIGH);
  analogWrite(PWMA, speed);
  setB(LOW, HIGH);
  analogWrite(PWMB, speed);
  dwh(STBY);
}

/* stop left motor, forward right motor */
inline void Movement::forwardleft(int speed) {
  setA(LOW, HIGH);
  analogWrite(PWMA, 0);
  setB(LOW, HIGH);
  analogWrite(PWMB, speed);
  dwh(STBY);
}

/* stop right motor, forward left motor */
inline void Movement::forwardright(int speed) {
  setA(LOW, HIGH);
  analogWrite(PWMA, speed);
  setB(LOW, HIGH);
  analogWrite(PWMB, 0);
  dwh(STBY);
}

/* turn both motors backwards */
inline void Movement::backward(int speed) {
  setA(HIGH, LOW);
  analogWrite(PWMA, speed);
  setB(HIGH, LOW);
  analogWrite(PWMB, speed);
  dwh(STBY);
}

/* stop right motor, backward left motor */
inline void Movement::backwardleft(int speed) {
  setA(HIGH, LOW);
  analogWrite(PWMA, speed);
  setB(HIGH, LOW);
  analogWrite(PWMB, 0);
  dwh(STBY);
}

/* stop left motor, backward right motor  */
inline void Movement::backwardright(int speed) {
  setA(HIGH, LOW);
  analogWrite(PWMA, 0);
  setB(HIGH, LOW);
  analogWrite(PWMB, speed);
  dwh(STBY);
}

/* turn motors alternatively to spin counterclockwise  */
inline void Movement::spinleft(int speed) {
  setA(HIGH, LOW);
  analogWrite(PWMA, speed);
  setB(LOW, HIGH);
  analogWrite(PWMB, speed);
  dwh(STBY);
}

/* turn motors alternatively to spin clockwise  */
inline void Movement::spinright(int speed) {
  setA(LOW, HIGH);
  analogWrite(PWMA, speed);
  setB(HIGH, LOW);
  analogWrite(PWMB, speed);
  dwh(STBY);
}


/*--------- helper ---------*/
inline void setPins(int pin1, int val1, int pin2, int val2) {
  digitalWrite(pin1, val1);
  digitalWrite(pin2, val2);
}
inline void setA(int ain1, int ain2) {
  setPins(AIN1, ain1, AIN2, ain2);
}

inline void setB(int bin1, int bin2) {
  setPins(BIN1, bin1, BIN2, bin2);
}

#endif
