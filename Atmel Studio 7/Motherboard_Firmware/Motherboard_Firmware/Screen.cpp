/*
* Screen.cpp
*
* Created: 4/3/2019 11:10:52 AM
* Author: Anthony
*/


#include "Screen.h"
#include "Queue.h"
#include "Structs.h"
#include "SerialProcessing.h"
#include "DataConversions.h"
#include "Error.h"

Screen *Screen::firstInstance;

bool previousHadError = false;

// default constructor
Screen::Screen()
{
	if (!firstInstance)
	{
		firstInstance = this;
	}
} //Screen

void Screen::init()
{
	
	
}

void Screen::UpdateDiameter(SpcDiameter *spcDiameter)
{
	SerialProcessing serialProcessing;
	SerialCommand serialCommand;

	serialCommand.command = "setDiameter";
	serialCommand.hardwareType = hardwareType.screen;
	serialCommand.value = spcDiameter->charDiameterNoDecimal;

	serialProcessing.SendScreenData(&serialCommand);

	serialCommand.command = "setDiameterDecimalLocation";
	serialCommand.hardwareType = hardwareType.screen;
	char buf[5] = {0};
	CONVERT_NUMBER_TO_STRING(INT_FORMAT, spcDiameter->decimalPointLocation, buf);
	serialCommand.value = buf;

	serialProcessing.SendScreenData(&serialCommand);
}

void Screen::UpdateSpool(RPM *rpm)
{
	SerialProcessing serialProcessing;
	SerialCommand serialCommand;

	serialCommand.command = "setSpoolRPM";
	serialCommand.hardwareType = hardwareType.screen;
	serialCommand.value = rpm->charRpm;

	serialProcessing.SendScreenData(&serialCommand);

}

void Screen::UpdatePuller(RPM *rpm)
{
	SerialProcessing serialProcessing;
	SerialCommand serialCommand;

	serialCommand.command = "setPullerRPM";
	serialCommand.hardwareType = hardwareType.screen;
	serialCommand.value = rpm->charRpm;

	serialProcessing.SendScreenData(&serialCommand);

}

void Screen::SendError(Error *error)
{
	SerialProcessing serialProcessing;
	SerialCommand serialCommand;

	

	serialCommand.command = "setDiameterError"; //this sends it to the "diameter slot" on the screen
	serialCommand.hardwareType = hardwareType.screen;
	serialCommand.value = error->errorDescription;

	serialProcessing.SendScreenData(&serialCommand);
}




void ConcantenateChars(char *buffer_A, char *buffer_B, char *outputBuffer)
{
	sprintf(outputBuffer, SCREENFORMAT, buffer_A, buffer_B);
}



void Screen::DisplayError(void)
{
	//for (int i = 0; i < 10; i++)
	//{
	//errors[i] = eError[i];
	//}

	//errors = *eError;
}

void Screen::ClearError(void)
{
	//error.errorLevel = 0;

}






// default destructor
Screen::~Screen()
{
} //~Screen
