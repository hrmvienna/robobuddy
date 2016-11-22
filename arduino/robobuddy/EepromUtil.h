/*
    HEADER TODO
*/

#ifndef EEpromUtil_h
#define EEpromUtil_h

#include <EEPROM.h>

#define CAL_EEPROM_ADDR 0

// prototypes - helper functions
inline void EeepromWriteInt(int address, int data);
inline int  EeepromReadInt(int address);

class EepromUtil
{
  public:
    EepromUtil();
    inline void writeArray(volatile int *data, int d_size);
    inline void readArray(volatile int *data, int d_size);
};

// constructor
EepromUtil::EepromUtil() {}

/* store an array in eeprom */
inline void EepromUtil::writeArray(volatile int *data, int d_size) {
  for (int i = 0; i < d_size; i++) {
    EeepromWriteInt(CAL_EEPROM_ADDR + (2*i), data[i]); 
  }
}

/* read an array from eeprom */
inline void EepromUtil::readArray(volatile int *data, int d_size) {
  for (int i = 0; i < d_size; i++) {
    data[i] = EeepromReadInt(CAL_EEPROM_ADDR + (2*i)); 
  }
}

/* store an two byte int at the given address */
inline void EeepromWriteInt(int address, int data) {
  byte low_byte = data & 0xFF;
  byte high_byte = (data >> 8) & 0xFF;

  EEPROM.write(address, low_byte);
  EEPROM.write(address + 1, high_byte);
}

/* read an two byte int form the given address */
inline int  EeepromReadInt(int address) {
  byte low_byte  = EEPROM.read(address);
  byte high_byte = EEPROM.read(address + 1);
  
  return (low_byte & 0xFF) + ((high_byte << 8) & 0xFF00);
}

#endif
