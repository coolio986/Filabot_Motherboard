/*
 * Error.h
 *
 * Created: 4/18/2019 9:20:54 AM
 *  Author: Anthony
 */ 


#ifndef ERROR_H_
#define ERROR_H_

//#include <Vector.h>
#include "Arduino.h"



typedef struct
{
	uint16_t hardwareType; //see hardwareTypes.h
	//char *command;
	char *errorDescription;
	byte errorLevel;
	byte errorCode;
} Error;

//char *ErrorCodes[];

//errorLevel:
// 1 = datastream validation failed
// 2 = device disconnected
// 3 = undefined

//errorCode:
// 1 = DIAMETER DEVICE DISCONNECTED
// 2 = SPC DATA ERROR


void startErrorHandler(void);
void AddError(Error *eError);
void ClearError(byte ErrorCode);
bool HasErrors(void);
bool HasErrorCode(byte code);
Error *GetErrorByCode(byte code);











#endif /* ERROR_H_ */