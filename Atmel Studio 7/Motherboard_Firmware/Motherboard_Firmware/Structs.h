/*
 * Structs.h
 *
 * Created: 5/23/2019 4:42:21 PM
 *  Author: Anthony
 */ 


#ifndef STRUCTS_H_
#define STRUCTS_H_

typedef struct
{
	uint16_t hardwareType; //see hardwareTypes.h
	char *command;
	char *value;
} SerialCommand;

typedef struct 
{
	char charDiameterNoDecimal[10];
	char charDiameterWithDecimal[10];
	int decimalPointLocation;
	uint16_t intDiameterNoDecimal;
	float floatDiameterWithDecimal;
} SpcDiameter;

typedef struct  
{
	char charRpm[10];
	int intRpm;
} RPM;







#endif /* STRUCTS_H_ */