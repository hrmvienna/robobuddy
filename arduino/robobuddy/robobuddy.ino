#include "Movement.h"
#include "LineSensors.h"
#include "EepromUtil.h"
#include "Leds.h"
#include <Wire.h>

// defines
#define I2C_ADDRESS 0x04

#define GIVE_UP   512
#define GIVE_UP_TIME 1000000 //1 sec
#define BACK_SEARCH_TIME 200000 //0.2 sec
#define TURN_START_TIME 250000 //0.25 sec
#define TURN_LEAVE_LINE_MARGIN 50
#define CAL_REPS  10

#define TURN_LEFT  true
#define TURN_RIGHT false
#define TURN_TIME 5
#define SENSOR_CNT 5
//#define DEBUG
#define DEBUG_2

// comands
#define CMD_CAL_LS_W  'w'
#define CMD_CAL_LS_B  'q'
#define CMD_CAL_LS_BD 't'

#define CMD_CHECK_STATUS 'c'
#define CMD_SEARCH_LINE 'a'
#define CMD_SEARCH_STATION 's'
#define CMD_EXIT_STATION 'd'
#define CMD_TURN_LEFT 'f'
#define CMD_TURN_RIGHT 'g'
#define CMD_TURN_AROUND 'j'
#define CMD_NEXT_STATION 'h'

#define CMD_GET_LS_L 'o'
#define CMD_GET_LS_R 'p'
#define CMD_GET_LS_ROAD 'i'

#define SPEED_DEF 0

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print (x)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
  #define DEBUG_LS_VAL(x, y) Serial.print(x); Serial.println(y, DEC)
  #define DEBUG_LR(x, y) DEBUG_LS_VAL("L :", x); DEBUG_LS_VAL("R: ", y)
  #define DEBUG_ST(x, y) DEBUG_LS_VAL("S1 :", x); DEBUG_LS_VAL("S2: ", y)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_LS_VAL(x, y)
  #define DEBUG_LR(x, y)
  #define DEBUG_ST(x, y)
#endif
#ifdef DEBUG_2
  #define DEBUG_ERR(x)  Serial.println (x)
#else
   #define DEBUG_ERR(x)  
#endif

// state machine
enum system_states { STATE_IDLE, STATE_START, STATE_BUSY, STATE_FIN, STATE_ERR };
volatile system_states sys_state = STATE_IDLE;
Leds leds;

enum spin_dir { SPIN_LEFT, SPIN_RIGHT };

// prototypes
bool searchLine(int speed);
inline bool searchStation();
bool searchStation(bool wait);
void exitStation(int speed);
bool turn(spin_dir dir);
inline void calibrateLineSensorsBounds();
inline void calibrateLineSensorsWhite();
inline void calibrateLineSensorsBlack();
void changeSpeedSet(int set);

// prototypes - helper functions
inline bool onLine ();
inline bool onLine (int val1, int val2);
inline bool onLine (int val1, int val2, int bound1, int bound2);
inline bool nearLine();
inline bool nearLine (int val1, int val2, int bound1, int bound2);
inline bool inStation();
inline bool inStation(int val);
inline bool inStation(int val, int bound);
inline bool roadAheadM();
inline bool roadAhead();
inline void changeState(system_states new_state);
inline void i2cSendData(int data);
inline void i2cSendArray(byte *data, int d_size);
inline int baTi(byte *data);

// Motor Control
Movement motorcontrol;
LineSensors linesensors;
EepromUtil eepromUtil;

//drive, drive_start, spin_l, spin_l_start, spin_r, spin_r_start, cor_pos
int motor_speed_sets[][7] =  {{130, 140, 110, 135, 120, 150, 110}, //slow with slower turn speed
                              {130, 140, 120, 135, 130, 150, 120}, //slow
                              {150, 160, 130, 145, 140, 150, 140}, //fast
                              {160, 170, 140, 155, 150, 160, 150}}; //very fast
volatile int motorspeed_drive = 0;
volatile int motorspeed_drive_start = 0;
volatile int motorspeed_spin_left = 0;
volatile int motorspeed_spin_right = 0;
volatile int motorspeed_spin_left_start = 0;
volatile int motorspeed_spin_right_start = 0;
volatile int motorspeed_cor_pos = 0;

char i2ccommand = ' ';
char i2ccurcmd = ' ';
int i2creturn = 0;

// sensor calibration
volatile int cal_w[SENSOR_CNT];
volatile int cal_b[SENSOR_CNT];
volatile int bounds[SENSOR_CNT];

// i2c status messages
byte I2C_ERR[]  = {0xEE, 0xEE};
byte I2C_BUSY[] = {0xBB, 0xBB};
byte I2C_OK[]   = {0xA1, 0xA1};
byte I2C_IDLE[] = {0x1D, 0x13};
byte I2C_TRUE[] = {0xB1, 0xB1};
byte I2C_FALSE[]= {0xB0, 0xB0};

/* setup: configure pins, initialize state, etc. */
void setup() {
  sys_state = STATE_IDLE;
  leds.idle();
  changeSpeedSet(SPEED_DEF);
  
  // initialize i2c as a slave
  Wire.begin(I2C_ADDRESS);
  // define callbacks for i2c communication
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);

  // read previous cal values from EEPROM
  eepromUtil.readArray(&bounds[0], SENSOR_CNT);

  #if defined(DEBUG) || defined(DEBUG_2)
  Serial.begin(9600);
  Serial.println("Ready");
  #endif
}

/* main loop */
void loop() {
  /* execute i2c command */
  if (sys_state == STATE_START) {
    changeState(STATE_BUSY);
    leds.busy();
    char i2ccmdtmp = i2ccommand;
    bool ret = false;
    i2ccurcmd = i2ccmdtmp;
    i2ccommand = ' ';
    DEBUG_PRINT("exec. cmd: ");
    DEBUG_PRINTLN(i2ccmdtmp);
    switch (i2ccmdtmp) {
      case CMD_CAL_LS_W:
        calibrateLineSensorsWhite();
        i2creturn = baTi(I2C_OK);
        changeState(STATE_FIN);
        leds.finished();
        break;
      case CMD_CAL_LS_B:
        calibrateLineSensorsBlack();
        i2creturn = baTi(I2C_OK);
        changeState(STATE_FIN);
        leds.finished();
        break;
      case CMD_CAL_LS_BD:
        calibrateLineSensorsBounds();
        i2creturn = baTi(I2C_OK);
        changeState(STATE_FIN);
        leds.finished();
        break;
      case CMD_SEARCH_LINE:
        ret = searchLine(motorspeed_drive);
        if (ret) {
          i2creturn = baTi(I2C_OK);
          changeState(STATE_FIN);
          leds.finished();
        } else {
          i2creturn = baTi(I2C_ERR);
          changeState(STATE_ERR);
          leds.error();
        }
        break;
      case CMD_SEARCH_STATION:
        ret = searchStation();
        if (ret) {
          i2creturn = baTi(I2C_OK);
          changeState(STATE_FIN);
          leds.finished();
        } else {
          i2creturn = baTi(I2C_ERR);
          changeState(STATE_ERR);
          leds.error();
        }
        break;
      case CMD_EXIT_STATION:
        exitStation(motorspeed_drive_start);
        i2creturn = baTi(I2C_OK);
        changeState(STATE_FIN);
        leds.finished();
        break;
      case CMD_TURN_LEFT:
        ret = turn(SPIN_LEFT);
        if (ret) {
          i2creturn = baTi(I2C_OK);
          changeState(STATE_FIN);
          leds.finished();
        } else {
          i2creturn = baTi(I2C_ERR);
          changeState(STATE_ERR);
          leds.error();
        }
        break;
      case CMD_TURN_RIGHT:
        ret = turn(SPIN_RIGHT);
        if (ret) {
          i2creturn = baTi(I2C_OK);
          changeState(STATE_FIN);
          leds.finished();
        } else {
          i2creturn = baTi(I2C_ERR);
          changeState(STATE_ERR);
          leds.error();
        }
        break;
      case CMD_NEXT_STATION:
        // check if road is ahead
        if (!roadAheadM()) {
          DEBUG_PRINTLN("roadAhead: no road ahead");
          DEBUG_ERR("No road ahead");
          i2creturn = baTi(I2C_ERR);
          changeState(STATE_ERR);
          leds.error();
          break;
        } 
        
        //if robot is in station, exit it first
        if (inStation() == true) {
          exitStation(motorspeed_drive_start);
        }
        ret = searchStation(true);
        if (ret) {
          i2creturn = baTi(I2C_OK);
          changeState(STATE_FIN);
          leds.finished();
        } else {
          i2creturn = baTi(I2C_ERR);
          changeState(STATE_ERR);
          leds.error();
        }
        break;
      case CMD_TURN_AROUND:
        ret = turn(SPIN_LEFT);
        if (ret) {
          delay(100);
          ret = turn(SPIN_LEFT);
          if (ret) {
            i2creturn = baTi(I2C_OK);
            changeState(STATE_FIN);
            leds.finished();
          } else {
            i2creturn = baTi(I2C_ERR);
            changeState(STATE_ERR);
            leds.error();
          }
        } else {
          i2creturn = baTi(I2C_ERR);
          changeState(STATE_ERR);
          leds.error();
        }
        break;
      case CMD_GET_LS_L:
        i2creturn = linesensors.getLeft();
        changeState(STATE_FIN);
        leds.finished();
        break;
      case CMD_GET_LS_R:
        i2creturn = linesensors.getRight();
        changeState(STATE_FIN);
        leds.finished();
        break;
      case CMD_GET_LS_ROAD:
        if (roadAheadM()) {
          i2creturn = baTi(I2C_TRUE);
        } else {
          i2creturn = baTi(I2C_FALSE);
        }
        changeState(STATE_FIN);
        leds.finished();
        break;
      default:
        if ((i2ccmdtmp >= '0') && (i2ccmdtmp <= '9')) {
          changeSpeedSet(i2ccmdtmp - '0');
          i2creturn = baTi(I2C_OK);
          changeState(STATE_FIN);
          leds.finished();
        } else {
          changeState(STATE_ERR);
          leds.error();
        }
        break;
    }
  }
  
  delay(100);
}

/* callback for i2c receive data */
void receiveData(int byteCount) {
  while (Wire.available()) {
    //DEBUG_PRINTLN("-----------------------");
    char i2ctmp = Wire.read();
    //DEBUG_PRINT("received: ");
    //DEBUG_PRINTLN(i2ccommand);

    if ((sys_state == STATE_IDLE) && (i2ctmp != CMD_CHECK_STATUS)) {
      if (i2ctmp != ' ') {
        i2ccommand = i2ctmp;
        sys_state = STATE_START;
      }
    }
  }
}

/* callback for i2c sending data */
void sendData(){
  byte tmpArray[2];
  //DEBUG_PRINT("send. cmd: ");
  //DEBUG_PRINTLN(i2ccurcmd);

  switch (sys_state) {
    case STATE_IDLE:  
      i2cSendArray(I2C_IDLE, 2);
      break;
    case STATE_START:
      /* Fall through */
    case STATE_BUSY:
      i2cSendArray(I2C_BUSY, 2);
      break;
    case STATE_FIN:
      i2cSendData(i2creturn);
      changeState(STATE_IDLE);
      leds.idle();
      break;
    case STATE_ERR:
      i2cSendArray(I2C_ERR, 2);
      changeState(STATE_IDLE);
      leds.idle();
      break;
    default:
      break;
  }
}

/* searchLine: turn the roboter until a line is found, or give up */
bool searchLine(int speed) {
  DEBUG_PRINTLN("start searchLine");
  // read line sensors
  volatile int sl_left = linesensors.getLeft();
  volatile int sl_right = linesensors.getRight();
  int reps = 0;
  DEBUG_LR(sl_left, sl_right);
  
  // check if the roboter is already on the line
  if (onLine(sl_left, sl_right)) {
    DEBUG_PRINTLN("already on line");
    return true;
  }

  // check if the roboter is near the line
  if (nearLine()) {
    DEBUG_PRINTLN("near line");

    sl_left = linesensors.getLeft();
    sl_right = linesensors.getRight();

    if (sl_left < bounds[0]) {
      motorcontrol.spinright(speed);
      do {
        sl_left = linesensors.getLeft();
      } while (sl_left < bounds[0]);
      motorcontrol.stop();
    } else {
      motorcontrol.spinleft(speed);
      do {
        sl_right = linesensors.getRight();
      } while (sl_right < bounds[1]);
      motorcontrol.stop();
    }
    
    motorcontrol.stop();
    DEBUG_LR(sl_left, sl_right);
    DEBUG_PRINTLN("line found");
    return true;
  } 
  motorcontrol.stop();
  
  // turn until line is found, or give up
  DEBUG_PRINTLN("search line");
  long start_time = micros();
  sl_left = linesensors.getLeft();
  sl_right = linesensors.getRight();
  // compare sensor values to determine in which direction to turn
  if (sl_left > sl_right) {
    //start search by turing right
    motorcontrol.spinright(speed);
  } else {
    //start search by turing left
    motorcontrol.spinleft(speed);
  }
  while (!onLine() && (micros() - start_time) < GIVE_UP_TIME) {}
  motorcontrol.stop();
  delay(100);
  
  if (onLine()) {
    DEBUG_LR(sl_left, sl_right);
    DEBUG_PRINTLN("line found");
    return true;
  } else {
    DEBUG_PRINTLN("gave up search");
    DEBUG_ERR("searchLine: gave up search");
    return false;
  }
}

/* searchStation: follow the line until a station is reached */
inline bool searchStation() {
  return searchStation(false);
}

/* searchStation: follow the line until a station is reached */
bool searchStation(bool wait) {
  DEBUG_PRINTLN("start searchStation");
  volatile int sl_left = 0;
  volatile int sl_right = 0;
  volatile int sl_s1 = 0;
  volatile int sl_s2 = 0;
  volatile int speed = motorspeed_drive;

  //check if roboter is on line
  if (!searchLine(speed)) {
    return false;
  }
  long start_time = micros();
  do {
    // read line sensors
    sl_left = linesensors.getLeft();
    sl_right = linesensors.getRight();

    if ((sl_left > bounds[0]) && (sl_right > bounds[1])) {
      motorcontrol.forward(speed);
    } else if ((sl_left > bounds[0]) && (sl_right <= bounds[1])) {
      motorcontrol.forwardleft(speed);
    } else if ((sl_left <= bounds[0]) && (sl_right > bounds[1])) {
      motorcontrol.forwardright(speed);
    } else if (!searchLine(speed)) {
      return false;
    }
    delay(TURN_TIME);
  } while (!inStation() || ( wait && (micros() - start_time) < 1000000)); //search at least for 1 seconds
  motorcontrol.stop();

  //check if robot stoped too late
  if (!inStation() && !onLine()) {
    do {
      motorcontrol.backward(speed);
    } while (!inStation() && !onLine());
  }
  motorcontrol.stop();
  DEBUG_PRINTLN("found station");
  
  return true;
}

/* exitStation: follow the line until the station is left */
void exitStation(int speed) {
  DEBUG_PRINTLN("start exitStation");
  volatile int sl_s1 = 0;
  volatile int sl_s2 = 0;
  
  do {
    motorcontrol.forward(speed);
    delay(TURN_TIME);
  } while (inStation());
  motorcontrol.stop();
  DEBUG_PRINTLN("left station");
}

/* turn: turn the robot into the given direction until 
   the next station position is reached */
bool turn(spin_dir dir) {
  DEBUG_PRINTLN("start turn");
  volatile int sl_left = 0;
  volatile int sl_right = 0;
  volatile int sl_s1 = linesensors.getStation();
  volatile int speed = motorspeed_spin_left;

  long long start_time = micros();
  if (dir == SPIN_LEFT) {
    motorcontrol.spinleft(motorspeed_spin_left_start);
    while ((micros() - start_time) < TURN_START_TIME || nearLine()) {}
    delay(TURN_LEAVE_LINE_MARGIN);
    motorcontrol.spinleft(motorspeed_spin_left);

     do {
      sl_right = linesensors.getRight();
    } while (sl_right < bounds[1]);
  } else {
    motorcontrol.spinright(motorspeed_spin_right_start);
    while ((micros() - start_time) < TURN_START_TIME || nearLine()) {}
    delay(TURN_LEAVE_LINE_MARGIN);
    motorcontrol.spinright(motorspeed_spin_right);

     do {
      sl_left = linesensors.getLeft();
    } while (sl_left < bounds[0]);
  }

  motorcontrol.stop();
  DEBUG_PRINTLN("finish turn");
  delay(100);
  
  bool bRes = false;

  if (inStation() && onLine()) { 
    DEBUG_PRINTLN("finish turn - in correct position");   
    return true;
  }
  
  DEBUG_PRINTLN("finish turn - adjust position");

  //search for station backwards 
  bool back_straight = true;
  spin_dir back_dir;
  start_time = micros();
  if (onLine()) {
    while(!inStation() && (micros() - start_time) < BACK_SEARCH_TIME) { //0.1 seconds
      motorcontrol.backward(motorspeed_cor_pos);
    }
    motorcontrol.stop();
  } else if (nearLine()) {
    back_straight = false;
    sl_left = linesensors.getLeft();
    if (sl_left < bounds[0]) {
      back_dir = SPIN_RIGHT;
      while(!inStation() && (micros() - start_time) < BACK_SEARCH_TIME) { //0.1 seconds
        motorcontrol.backwardright(motorspeed_cor_pos);
      }
      motorcontrol.stop();
    } else {
     back_dir = SPIN_LEFT;
      while(!inStation() && (micros() - start_time) < BACK_SEARCH_TIME) { //0.1 seconds
        motorcontrol.backwardleft(motorspeed_cor_pos);
      }
      motorcontrol.stop();
    }
  } else {
    DEBUG_ERR("turn 0: after turn not near line");
    return false;
  }
  
  delay(100);
  if (inStation() && onLine()) {
    DEBUG_PRINTLN("correct position found - backwards");
    return true;
  }

  DEBUG_ERR("turn 1: didn't find station backwards");
  //now search for line again
  if (!onLine() && !nearLine()) {
    if (back_straight) {
      DEBUG_ERR("turn 1a: lost line after going straight backwards");
      return false;
    } else {
      if (back_dir == SPIN_LEFT) {
        //line sensors should be left of the line
        motorcontrol.spinright(motorspeed_spin_right);
      } else {
        // line sensors should be right of the line
        motorcontrol.spinleft(motorspeed_spin_left);
      }
      while (!onLine()) {} 
      motorcontrol.stop();
      DEBUG_ERR("turn 1b: should now be on line");
    }
  }
  if (!onLine() && nearLine()) {
    sl_left = linesensors.getLeft();
    sl_right = linesensors.getRight();

    if (sl_left < bounds[0]) {
      //left sensor is not on line
      motorcontrol.spinright(motorspeed_spin_right);
      do {
        sl_left = linesensors.getLeft();
      } while (sl_left < bounds[0]);
    } else {
      //rihgt sensor is not on line
      motorcontrol.spinleft(motorspeed_spin_left);
      do {
        sl_right = linesensors.getRight();
      } while (sl_right < bounds[1]);
    }
    motorcontrol.stop();
  } 

  DEBUG_ERR("turn 2a: should be in onLine");
  delay(100);

  if(!onLine()) {
    DEBUG_ERR("turn 2b: not on line");
  }
  if(nearLine()) {
    DEBUG_ERR("turn 2c: near line");
  }

  //search for station forwards
  if(!inStation()) {
    DEBUG_PRINTLN("finish turn - search for station");
    bRes = searchStation();
    motorcontrol.stop();
    return bRes;
  } 

  DEBUG_ERR("turn 3a: should be in station and onLine");
  delay(100);
  if (!onLine() && nearLine()) {
    DEBUG_ERR("turn 3b: correct position, but only near line");
    return searchLine(motorspeed_drive);
  }
  DEBUG_PRINTLN("finish turn - in correct position");
  return true;
  
}

/* calibrateLineSensorsBounds: store and calulate the line sensor boundaries, using the 
   read values form calibrateLineSensorsWhite() and calibrateLineSensorsBlack() */
inline void calibrateLineSensorsBounds() {
  DEBUG_PRINTLN("calibrate sensor bounds:");
  for (int i=0; i < SENSOR_CNT; i++) {
    bounds[i] = (cal_w[i] + cal_b[i])/2;
  }

  // store cal values in EEPROM
  eepromUtil.writeArray(&bounds[0], SENSOR_CNT);

  DEBUG_PRINTLN("calc LS bounds");
  DEBUG_LR(bounds[0], bounds[1]);
  DEBUG_ST(bounds[2], bounds[3]);
  DEBUG_PRINTLN("------------------");
}

/* calibrateLineSensorsWhite: read the line sensor values for white surfaces */
inline void calibrateLineSensorsWhite() {
  cal_w[0] = linesensors.getLeft();
  cal_w[1] = linesensors.getRight();
  cal_w[2] = linesensors.getStation();
  cal_w[3] = linesensors.getRoad1();
  cal_w[4] = linesensors.getRoad2();
  
  DEBUG_PRINTLN("calibrate LS white");
  DEBUG_LR(cal_w[0], cal_w[1]);
  DEBUG_ST(cal_w[2], cal_w[3]);
  DEBUG_PRINTLN("------------------");
}

/* calibrateLineSensorsBlack: read the line sensor values for black surfaces */
inline void calibrateLineSensorsBlack() {
  cal_b[0] = linesensors.getLeft();
  cal_b[1] = linesensors.getRight();
  cal_b[2] = linesensors.getStation();
  cal_b[3] = linesensors.getRoad1();
  cal_b[4] = linesensors.getRoad2();

  DEBUG_PRINTLN("calibrate LS black");
  DEBUG_LR(cal_b[0], cal_b[1]);
  DEBUG_ST(cal_b[3], cal_b[4]);
  DEBUG_PRINTLN("------------------");
}

/* change speed set */
void changeSpeedSet(int set) {
  int i = 0;

  if ((set >= 0) && (set <= 2)){
    i = set;
  } 

  //drive, drive_start, spin_l, spin_l_start, spin_r, spin_r_start, cor_pos
  motorspeed_drive            = motor_speed_sets[i][0];
  motorspeed_drive_start      = motor_speed_sets[i][1];
  motorspeed_spin_left        = motor_speed_sets[i][2];
  motorspeed_spin_left_start  = motor_speed_sets[i][3];
  motorspeed_spin_right       = motor_speed_sets[i][4];
  motorspeed_spin_right_start = motor_speed_sets[i][5];
  motorspeed_cor_pos          = motor_speed_sets[i][6];
}

/*--------- helper ---------*/
inline bool onLine () {
  return onLine(linesensors.getLeft(), linesensors.getRight());
}

inline bool onLine (int val1, int val2) {
  return onLine(val1, val2, bounds[0], bounds[1]);
}

inline bool onLine (int val1, int val2, int bound1, int bound2) {
  if ((val1 > bound1) && (val2 > bound2)) {
    return true;
  }
  return false;
}

inline bool nearLine() {
  return nearLine(linesensors.getLeft(), linesensors.getRight(), bounds[0], bounds[1]); 
}

inline bool nearLine (int val1, int val2, int bound1, int bound2) {
  if ((val1 > bound1) || (val2 > bound2)) {
    return true;
  }
  return false;
}

inline bool inStation() {
  return inStation(linesensors.getStation());
}

inline bool inStation(int val) {
  return inStation(val, bounds[2]);
}

inline bool inStation(int val, int bound) {
  if (val > bound) {
    return true;
  }
  return false;
}

inline bool roadAheadM() {
  bool bRes = false;
  for (int i=0; i<10; i++) {
    bRes = bRes || roadAhead();
  }
  return bRes;
}

inline bool roadAhead() {
  if ((linesensors.getRoad1() > bounds[3]) || (linesensors.getRoad2() > bounds[4])) {
    return true;
  }
  return false;
}

inline void i2cSendData(int data) {
  byte tmpArray[2];
  tmpArray[0] = (data >> 8) & 0xFF;
  tmpArray[1] = data & 0xFF;
  i2cSendArray(tmpArray, 2);
}

inline void changeState(system_states new_state) {
  //DEBUG_PRINTLN("------------------");
  //DEBUG_PRINT("chg state: ");
  //DEBUG_PRINTLN(new_state);
  sys_state = new_state;
}

inline void i2cSendArray(byte *data, int d_size) {
  Wire.write(data, d_size);
}


inline int baTi(byte *data) {
  return (data[1] << 8) + data[0];
}

