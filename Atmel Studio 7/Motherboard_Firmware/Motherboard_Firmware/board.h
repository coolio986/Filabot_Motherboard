#ifndef __BOARD_H__
#define __BOARD_H__

#include <Arduino.h>
#include "HardwareTypes.h"

#define SERIAL_BAUD (115200) //baud rate for the serial ports

#define INDICATOR_REQ  30 //when set high the CLK pulse train starts
#define INDICATOR_DAT  28
#define INDICATOR_CLK  2//29  CLK is a pulse train provided by the indicator

#define MYHARDWARETYPE INTERNALDEVICE

#define MAX_CMD_LENGTH 60
#define DELIMITER ";"

extern bool SIMULATIONACTIVE;

#endif//__BOARD_H__

