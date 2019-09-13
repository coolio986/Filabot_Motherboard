#ifndef __BOARD_H__
#define __BOARD_H__

#include <Arduino.h>
#include "HardwareTypes.h"
#include "SerialNative.h"

#define SERIAL_BAUD (115200) //baud rate for the serial ports

#define INDICATOR_REQ  30 //when set high the CLK pulse train starts
#define INDICATOR_DAT  28
#define INDICATOR_CLK  2//29  CLK is a pulse train provided by the indicator
#define ENCODER_PINA   26
#define ENCODER_PINB   24
#define ENCODER_PB	   51
#define DIAMETER05		31
#define DIAMETER420		33
#define START_PB		47
#define STOP_PB			49

#define MYHARDWARETYPE INTERNALDEVICE

#define MAX_CMD_LENGTH 60
#define DELIMITER ";"

extern bool SIMULATIONACTIVE;
extern _SerialNative SerialNative;

#endif//__BOARD_H__

