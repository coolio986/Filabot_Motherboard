/*===========================================================
* Project: Motherboard_Firmware
* Developed by: Anthony Kaul(3D Excellence LLC) in collaboration with Filabot (Triex LLC)
* This firmware converts SPC (Statistical Process Control) into signals that can be recognized by a PC
* This firmware also allows for communication to various devices via serial and coordinates all items
* All rights reserved


* Version information:
* v1.2 - beta
* ===========================================================*/

// ***** INCLUDES ***** //
#include <Arduino.h>
#include <FreeRTOS_ARM.h>
#include "SerialProcessing.h"
#include "hardwareTypes.h"
#include "SpcProcessing.h"
#include "Screen.h"
#include "board.h"
#include "Error.h"
#include "DataConversions.h"
#include "SerialNative.h"
#include <Encoder.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
// ***** INCLUDES ***** //

// ***** FreeRTOS  ***** //
#define INCLUDE_vTaskDelay   1
#define configUSE_PREEMPTION 1
// Redefine AVR Flash string macro as nop for ARM
#undef F
#define F(str) str
// ***** FreeRTOS  ***** //


// ***** TASKS **** //
#define TASKSCREEN
#define TASKSPC
#define TASKSERIALCOMMANDS
//#define TASKPULLER
//#define TASKTRAVERSE
#define ENCODER
// ***** TASKS **** //



// **** PROTOTYPES **** //
void CheckSerialCommands();
void RunSPCDataLoop();
int CheckInteralCommands(char* code);
int CheckSpoolerCommands(char* code);
int PrintRandomDiameterData();
void PrintRandomRPMData();
bool startsWith(const char* pre, const char* str);
void TaskCheckSPC( void *pvParameters );
void TaskCheckSerialExpander( void *pvParameters );
void TaskSendToScreen ( void *pvParameters );
void TaskCheckSerialCommands( void *pvParameters );
void TaskRunSimulation(void *pvParameters );
void TaskGetPullerRPM (void *pvParameters);
void TaskUpdateTraverse (void *pvParameters);
void TaskCheckEncoder (void *pvParameters);
void checkSPC();
// **** PROTOTYPES **** //



// These are used to get information about static SRAM and flash memory sizes
extern "C" char __data_start[];    // start of SRAM data
extern "C" char _end[];     // end of SRAM data (used to check amount of SRAM this program's variables use)
extern "C" char __data_load_end[];  // end of FLASH (used to check amount of Flash this program's code and data uses)

// declare since extern in board.h
bool SIMULATIONACTIVE = false; //sets default value for simulation
_SerialNative SerialNative;


// ***** CLASS DECLARATIONs **** //
SerialCommand sCommand;
SpcProcessing spcProcessing;
Screen screen;
SerialProcessing serialProcessing;
SemaphoreHandle_t xSemaphore = NULL;
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
#ifdef ENCODER
Encoder screenEncoder(ENCODER_PINA, ENCODER_PINB);
#endif
// ***** CLASS DECLARATIONs **** //


float pullerRPM = 0;
int32_t previousEncoderValue = 0;
unsigned long previousMillis = 834000;
int toggle = 0;


void setup()
{
	
	
	
	//int usbConnectionRetries = 10;
	//while (!SerialNative && usbConnectionRetries > 0)
	//{
	//delay(100);
	//usbConnectionRetries--;
	//}
	//if (usbConnectionRetries > 0){
	SerialNative.begin(SERIAL_BAUD); //using native serial rather than programming port on DUE
	SerialNative.setTimeout(1);

	ads.begin();
	ads.setGain(GAIN_ONE);

	pinMode(ENCODER_PB, INPUT_PULLUP);
	pinMode(START_PB, INPUT_PULLUP);
	pinMode(STOP_PB, INPUT_PULLUP);

	//}
	//SerialUSB.begin(SERIAL_BAUD); //using native serial rather than programming port on DUE
	Serial3.begin(SERIAL_BAUD); //Serial 3 for communication with external screen ILI9341
	
	// **** INITS ***** //
	spcProcessing.init();
	screen.init();
	serialProcessing.init();
	startErrorHandler();
	// **** INITS ***** //

	// ***** Instances ***** //
	xSemaphore = xSemaphoreCreateMutex();

	
	

	#ifdef TASKSCREEN
	xTaskCreate(
	TaskSendToScreen
	,  (const portCHAR *)"SendToScreen"   // A name just for humans
	,  1500  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,   2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	#endif

	#ifdef TASKSPC
	xTaskCreate(
	TaskCheckSPC
	,  (const portCHAR *)"CheckSPC"   // A name just for humans
	,  1000  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,   1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	#endif
	
	#ifdef TASKSERIALCOMMANDS
	xTaskCreate(
	TaskCheckSerialCommands
	,  (const portCHAR *)"CheckSerialCommands"   // A name just for humans
	,  1000  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	#endif

	#ifdef TASKPULLER
	xTaskCreate(
	TaskGetPullerRPM
	,  (const portCHAR *)"GetPullerRPM"   // A name just for humans
	,  1000  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	#endif

	#ifdef TASKTRAVERSE
	xTaskCreate(
	TaskUpdateTraverse
	,  (const portCHAR *)"UpdateTraverse"   // A name just for humans
	,  1000  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	#endif

	#ifdef ENCODER
	xTaskCreate(
	TaskCheckEncoder
	,  (const portCHAR *)"CheckEncoder"   // A name just for humans
	,  500  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	#endif

	// ***** Instances ***** //

	vTaskStartScheduler(); //start FreeRTOS scheduler
	SerialNative.println("Insufficient RAM"); //code execution should never get here, but could if there is not enough RAM
	while(1); //hang processor on error
}

void loop()
{
	//nothing to do here, all routines done in tasks

	
}


void TaskSendToScreen(void *pvParameters)  // This is a task.
{
	while(1) // A Task shall never return or exit.
	{
		if ( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
		{
			TickType_t xLastWakeTime;
			xLastWakeTime = xTaskGetTickCount();

			if (spcProcessing.HasError())
			{
				screen.SendError(spcProcessing.GetError());  //need to implement later
			}
			if (spcProcessing.HasNewData)
			{
				//SerialUSB.println(spcProcessing.GetDiameter()->charDiameterWithDecimal); //Serial print is broken using long values, use char instead
				//SerialCommand _serialCommand;
				//_serialCommand.hardwareType = hardwareType.screen;
				//_serialCommand.command = "Diameter";
				//_serialCommand.value = spcProcessing.GetDiameter()->charDiameterWithDecimal;
				//char output[MAX_CMD_LENGTH] = {0};
				//BuildSerialOutput(&_serialCommand, output);
				//Serial3.println(output);
				
				//long running task
				screen.UpdateDiameter(spcProcessing.GetDiameter());
				
			}




			
			

			//char randomNum[10] = {0};
			//CONVERT_NUMBER_TO_STRING(INT_FORMAT, random(0, 1000), randomNum); //random for now
			//Spool spool;
			//spool.RPM = randomNum;

			//screen.UpdateSpool(&spool);
			//Puller puller;
			
			//char randomNum2[10] = {0};

			//CONVERT_NUMBER_TO_STRING(INT_FORMAT, random(0, 1000), randomNum2); //random for now
			//puller.RPM = randomNum2;
			//puller.RPM = "500";
			
			//screen.UpdatePuller(&puller);
			

			xSemaphoreGive(xSemaphore);
			vTaskDelayUntil( &xLastWakeTime, 50);

		}

	}
}

void TaskCheckSPC(void *pvParameters)  // This is a task.
{

	while(1) // A Task shall never return or exit.
	{
		if ( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
		{
			TickType_t xLastWakeTime;
			xLastWakeTime = xTaskGetTickCount();


			spcProcessing.StartQuery();//enable interrupts and start the bit gathering from spc
			
			while (!spcProcessing.ISR_READY()) //need to remove this while loop
			{
				if (spcProcessing.QueryFailed())
				{
					break;
				}
				//if (spcProcessing.GetLoopCounts() > 50000)
				//{
				//
				//spcProcessing.StopQuery();
				//break;
				//}

				
			}
			
			
			spcProcessing.RunSPCDataLoop(); //once ISR is ready we can collect the data and build an output, needs to be done before query is stopped
			spcProcessing.StopQuery(); // stop query, kill clk interrupt on SPC
			
			
			if (spcProcessing.HasNewData)
			{
				//SerialUSB.println(spcProcessing.GetDiameter()->charDiameterWithDecimal); //Serial print is broken using long values, use char instead
				SerialCommand _serialCommand;
				_serialCommand.hardwareType = hardwareType.indicator;
				_serialCommand.command = "Diameter";
				_serialCommand.value = spcProcessing.GetDiameter()->charDiameterWithDecimal;
				char output[MAX_CMD_LENGTH] = {0};
				
				BuildSerialOutput(&_serialCommand, output);
				SerialNative.println(output);
				
			}
			
			
			xSemaphoreGive(xSemaphore);
			vTaskDelayUntil( &xLastWakeTime, 50);
		}
		
	}

}

void TaskCheckSerialCommands(void *pvParameters)  // This is a task.
{
	while(1) // A Task shall never return or exit.
	{
		if ( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
		{
			TickType_t xLastWakeTime;
			xLastWakeTime = xTaskGetTickCount();

			serialProcessing.Poll();
			xSemaphoreGive(xSemaphore);
			vTaskDelayUntil( &xLastWakeTime, 10);
		}
	}
}

void TaskGetPullerRPM(void *pvParameters)  // This is a task.
{
	while(1) // A Task shall never return or exit.
	{
		if ( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
		{
			TickType_t xLastWakeTime;
			xLastWakeTime = xTaskGetTickCount();

			SerialCommand serialCommand;
			serialCommand.command = "getrpm";
			serialCommand.hardwareType = hardwareType.puller;
			serialCommand.value = NULL;
			
			
			serialProcessing.SendDataToDevice(&serialCommand);
			delay(10);
			serialProcessing.CheckSerial(&Serial1, serialCommand.hardwareType);
			

			xSemaphoreGive(xSemaphore);
			vTaskDelayUntil( &xLastWakeTime, 50);
		}

		
	}

}

void TaskUpdateTraverse(void *pvParameters)  // This is a task.
{
	while(1) // A Task shall never return or exit.
	{
		if ( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
		{
			TickType_t xLastWakeTime;
			xLastWakeTime = xTaskGetTickCount();

			SerialCommand serialCommand;
			serialCommand.command = "SpoolRpm";
			serialCommand.hardwareType = hardwareType.traverse;
			serialCommand.value = NULL;
			
			
			//serialProcessing.SendDataToDevice(&serialCommand);
			//serialProcessing.CheckSerial(&Serial1, serialCommand.hardwareType);
			
			
			xSemaphoreGive(xSemaphore);
			vTaskDelayUntil( &xLastWakeTime, 50);
		}

		
	}

}

void TaskCheckEncoder(void *pvParameters)  // This is a task.
{
	while(1) // A Task shall never return or exit.
	{
		if ( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
		{
			TickType_t xLastWakeTime;
			xLastWakeTime = xTaskGetTickCount();

			#ifdef ENCODER
			//SerialNative.println(screenEncoder.read());
			int32_t encoderValue = screenEncoder.read();
			if (previousEncoderValue != encoderValue)
			{
				//if (previousEncoderValue > encoderValue)
				//{
				//pullerRPM--;
				//}
				//else
				//{
				//pullerRPM++;
				//}
				previousEncoderValue = encoderValue;
				SerialCommand sCommand;
				sCommand.hardwareType = hardwareType.puller;
				sCommand.command = "velocity";

				//float* fValue = &pullerRPM;
				//*fValue = *fValue * -1;
				
				char decimalNumber[20] = {0};
				CONVERT_FLOAT_TO_STRING(encoderValue, decimalNumber);

				sCommand.value = decimalNumber;
				//SerialNative.println(encoderValue);

				serialProcessing.SendDataToDevice(&sCommand);
			}
			
			#endif

			//if ((millis() - previousMillis) >= 834000 )
			//{
				//SerialCommand traverseCommand;
				//traverseCommand.hardwareType = hardwareType.traverse;
				//traverseCommand.command = "moveAbsolute";
//
				//if ((toggle % 2) == 0)
				//{
					//traverseCommand.value = "120000";
					//toggle++;
				//}
				//else
				//{
					//traverseCommand.value = "0";
					//toggle = 0;
				//}
				//previousMillis = millis();
				//serialProcessing.SendDataToDevice(&traverseCommand);
				//
			//}
			//SerialNative.print("PinA: ");
			//SerialNative.print(pina);
			//SerialNative.print("    PinB: ");
			//SerialNative.println(pinb);
			
			//int16_t adc1;
			//adc1 = ads.readADC_SingleEnded(0);
			//SerialNative.print("AIN1: ");
			//
			//SerialNative.println(adc1);
			
			
			
			xSemaphoreGive(xSemaphore);
			vTaskDelayUntil( &xLastWakeTime, 50);
		}

		
	}

}







