/*
* Device_Configuration.h
*
* Created: 3/12/2019 5:42:01 PM
* Author: Anthony
*/


#ifndef __DEVICE_CONFIGURATION_H__
#define __DEVICE_CONFIGURATION_H__

#include "HardwareTypes.h"

#define MYHARDWARETYPE INTERNALDEVICE

#define MAX_CMD_LENGTH 60
#define DELIMITER ";"

#define HOMING_SWITCH_USES_SPRING_RETURN false
#define HOMING_USES_DOUBLE_BUMP false
#define USE_POT_FOR_TRAVERSE  //uncomment to disable external speed pot

#define ADC_MIN_VALUE 0
#define ADC_MAX_VALUE 1023 //zero based

extern bool SIMULATIONACTIVE;

#endif //__DEVICE_CONFIGURATION_H__