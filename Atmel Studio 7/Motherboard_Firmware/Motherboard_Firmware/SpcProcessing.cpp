/*
* spcProcessing.cpp
*
* Created: 12/20/2018 4:57:59 PM
* Author: Anthony
*/


#include "SpcProcessing.h"
#include <Arduino.h>
#include "hardwareTypes.h"
#include "DataConversions.h"
#include "SerialProcessing.h"
#include "Error.h"
#include "Structs.h"

SpcProcessing *SpcProcessing::firstinstance;

void ISR_SPC();

// default constructor main entry point
SpcProcessing::SpcProcessing()
{
	if(!firstinstance)
	{
		firstinstance = this;
	}

	
} //spcProcessing

volatile int numberErrors;
long debugTime = 0;


volatile bool SPC_ISR_LOCK = false;
volatile char rawSPC_ISR[53] = {0};
volatile int ISR_LOOP_COUNTER = 0;
volatile int MAIN_LOOP_COUNTER = 0;
volatile char rawSPC[53] = {0};


void SpcProcessing::init(void)
{
	pinMode(INDICATOR_REQ, OUTPUT);
	pinMode(INDICATOR_CLK, INPUT_PULLUP);
	pinMode(INDICATOR_DAT, INPUT);

	
	//attachInterrupt(digitalPinToInterrupt(INDICATOR_CLK), ISR_SPC, FALLING);
	
	StartQuery();
}

void ISR_SPC()
{
	
	
	//rawSPC_ISR[ISR_LOOP_COUNTER++] = (PINA & digitalPinToBitMask(INDICATOR_DAT)) == 0 ? 48 : 49; //Keep track of ISR bits
	rawSPC_ISR[ISR_LOOP_COUNTER++] = digitalRead(INDICATOR_DAT) == 0 ? 48 : 49;
	//rawSPC_ISR[ISR_LOOP_COUNTER++] = PIOD->PIO_PDSR & PIO_PDSR_P3 == 0 ? 48 : 49;
	

	if (ISR_LOOP_COUNTER >= 52)
	{
		detachInterrupt(digitalPinToInterrupt(INDICATOR_CLK));
		digitalWrite(INDICATOR_REQ, LOW);
		
		
		//ISR_LOOP_COUNTER = 0; //set loop counter back to 0
		for (int i = 0; i < 52; i++)
		{
			rawSPC[i] = rawSPC_ISR[i]; //copy voltaile memory to non-volatile and clear the volatile to syncronize the main program loop
			rawSPC_ISR[i] = 0;
		}
		
		SPC_ISR_LOCK = false; //unlock ISR to synchronize main program loop
		
	}
	
	
}

void SpcProcessing::MainLoop(void)
{
	//if ((MAIN_LOOP_COUNTER > ISR_LOOP_COUNTER + 100)){ //track the loops to see if the ISR is still firing. Sometimes it doesn't trigger properly due to the screen updates
	
	
	//eError.hardwareType = INDICATOR;
	//
	//eError.errorLevel = 2;
	//eError.errorCode = 1;
	//AddError(&eError);

	//ISR_LOOP_COUNTER = 0;
	//MAIN_LOOP_COUNTER = 0;
	//SPC_ISR_LOCK = false;
	
	//digitalWrite(INDICATOR_REQ, LOW);
	//digitalWrite(INDICATOR_REQ, HIGH);

	
	
	//}

	//MAIN_LOOP_COUNTER++;
}

void SpcProcessing::RunSPCDataLoop(void)
{
	
	
	if (IsInSimulationMode)
	{
		PrintRandomDiameterData();
		return;
	}


	//if (MAIN_LOOP_COUNTER > 52 /*ISR_LOOP_COUNTER + 10*/){ //track the loops to see if the ISR is still firing. Sometimes it doesn't trigger properly due to the screen updates
	
	
	//eError.hardwareType = INDICATOR;
	//
	//eError.errorLevel = 2;
	//eError.errorCode = 1;
	//AddError(&eError);
	//
	//digitalWrite(INDICATOR_REQ, LOW);
	//digitalWrite(INDICATOR_REQ, HIGH);
	//
	//
	//ISR_LOOP_COUNTER = 0;
	//MAIN_LOOP_COUNTER = 0;
	//}
	
	//MAIN_LOOP_COUNTER++;

	if (!SPC_ISR_LOCK)
	{
		//Serial.println(rawSPC);

		if (rawSPC[0] == 0){ //if position 0 in array equals null then skip
			//Serial.println("RESETTING FROM NO DATA");
			//digitalWrite(INDICATOR_REQ, HIGH);
			//PORTC |= digitalPinToBitMask(INDICATOR_REQ); //set req high to restart ISR
			return;
		}

		

		bool dataStreamValid = false;
		for (unsigned int i = 0; i < 12; i++) //first 12 indcies should be 1's, if not then the data isn't valid
		{
			if (rawSPC[i] == 48 || rawSPC[16] == 49) //48 is 0 (zero) in ascii 0-12 cannot be 0, and 13 cannot be 1
			{
				dataStreamValid = false;
				
				SerialCommand sError;
				sError.hardwareType = INDICATOR;
				sError.command = "INDICATOR";
				sError.value = "A SPC PROCESSING ERROR HAS OCCURRED";
				
				numberErrors++;
				eError.hardwareType = INDICATOR;
				//eError.errorValue = "SPC DATA ERROR";
				eError.errorLevel = 1;
				eError.errorCode = 2;
				AddError(&eError);

				char sErrorOutput [MAX_CMD_LENGTH] = {0};
				BuildSerialOutput(&sError, sErrorOutput);
				SerialUSB.println(sErrorOutput);
				SPCDiameter = 0.00;
				ISR_LOOP_COUNTER = 0;
				break;
			}
			else
			{
				dataStreamValid = true;
			}
		}

		if (dataStreamValid)
		{
			

			ClearError(1);
			ClearError(2);
			byte bytes[13] = {0};
			for (int i = 0; i < 13; i++)
			{
				int idx = (i*4) + 4;
				int bitPointer = 0;
				for (int j = i * 4; j < idx ; j++)
				{
					bitWrite(bytes[i], bitPointer, rawSPC[j] == 49); //49 ascii for 1 //grab nibbles from rawSPC
					bitPointer++;
				}
			}
			
			float preDecimalNumber = 0.0;
			char buf[7];
			
			for(int i=0;i<6;i++){ //grab array positions 5-10 for diameter numbers
				
				buf[i]=bytes[i+5]+'0';
				
				buf[6]=0;
				
				preDecimalNumber=atol(buf); //assembled measurement, no decimal place added
			}
			
			int decimalPointLocation = bytes[11];
			
			
			SPCDiameter = preDecimalNumber / (pow(10, decimalPointLocation)); //add decimal to number
			//SPCDiameter = 1.75;
			
			
			
			spcDiameter.decimalPointLocation = decimalPointLocation;
			spcDiameter.intDiameterNoDecimal = preDecimalNumber;
			spcDiameter.floatDiameterWithDecimal = SPCDiameter;
			itoa(preDecimalNumber, spcDiameter.charDiameterNoDecimal, 10);
			
			
			char decimalNumber[20] = {0};
			CONVERT_FLOAT_TO_STRING(SPCDiameter, decimalNumber);
			CONVERT_FLOAT_TO_STRING(SPCDiameter, spcDiameter.charDiameterWithDecimal);


			//Serial.println(SPCDiameter);
			
			
			SerialCommand sCommand;
			sCommand.hardwareType = INDICATOR;
			sCommand.command = "INDICATOR";
			sCommand.value = decimalNumber;
			
			BuildSerialOutput(&sCommand, serialOutputBuffer);
			newData = true;

			for (int i = 0; i < 52; i++)
			{
				rawSPC[i] = 0;
			}
			
			MAIN_LOOP_COUNTER = 0;
			ISR_LOOP_COUNTER = 0;
		}

	}
	
}

char *SpcProcessing::GetSerialOutputBuffer(void)
{
	newData = false;
	return serialOutputBuffer;
}

SpcDiameter *SpcProcessing::GetDiameter(void){

	return &spcDiameter;
	//return SPCDiameter;
}

int SpcProcessing::GetLoopCounts(void)
{
	MAIN_LOOP_COUNTER++;
	return MAIN_LOOP_COUNTER;
}

bool SpcProcessing::QueryFailed(void)
{
	if (GetLoopCounts() > 50000) //50000 ticks before a failure is recorded (maybe time base is better?)
	{
		StopQuery();
		SerialUSB.println("Query Error");
		eError.hardwareType = INDICATOR;
		eError.errorLevel = 2;
		eError.errorCode = 1;
		AddError(&eError);
		
		return true;
	}
	return false;
}

bool SpcProcessing::HasError(void){

	return eError.errorLevel > 0;
}

Error *SpcProcessing::GetError(void){


	return &eError;
}

int SpcProcessing::PrintRandomDiameterData(void)
{
	char diameter[5];
	ltoa(random(17000, 18000), diameter, 10);
	
	SerialUSB.print("0;111111111111111111110000");
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			SerialUSB.print(((byte)diameter[i] >> j) & 1);
		}
	}
	SerialUSB.println("00100000");
	return 0;
}
bool SpcProcessing::ISR_READY(void)
{
	
	return !SPC_ISR_LOCK;
}

void SpcProcessing::StartQuery(void)
{

if (millis() > debugTime + 5000)
{
	SerialUSB.print("Number of errors: ");
	SerialUSB.println(numberErrors);
	debugTime = millis();
}
	
	SPC_ISR_LOCK = true; //lock ISR so main program loop doesn't interrupt
	attachInterrupt(digitalPinToInterrupt(INDICATOR_CLK), ISR_SPC, FALLING);
	//for (int i = 0; i < 10; i++){
	delay(2);
	
	//}
	ISR_LOOP_COUNTER = 0;
	MAIN_LOOP_COUNTER = 0;
	
	newData = false;
	digitalWrite(INDICATOR_REQ, HIGH);
	
	
	
}

void SpcProcessing::StopQuery(void)
{
	detachInterrupt(digitalPinToInterrupt(INDICATOR_CLK));
	
	digitalWrite(INDICATOR_REQ, LOW);
}


// default destructor
SpcProcessing::~SpcProcessing()
{
} //~spcProcessing


