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
//#define TASKSENDTOSCREEN
#define TASKCHECKSPC
#define TASKCHECKSERIALCOMMANDS
//#define TASKGETPULLERRPM
#define TASKGETSPOOLRPM
//#define TASKUPDATETRAVERSE
#define TASKCHECKENCODER
#define TASKGETFULLUPDATE
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
void TaskGetSpoolRPM (void *pvParameters);
void TaskUpdateTraverse (void *pvParameters);
void TaskCheckEncoder (void *pvParameters);
void TaskGetFullUpdate (void *pvParameters);
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
#ifdef TASKCHECKENCODER
Encoder screenEncoder(ENCODER_PINA, ENCODER_PINB);
#endif
// ***** CLASS DECLARATIONs **** //


float pullerRPM = 0;
int32_t previousEncoderValue = 0;
unsigned long previousMillis = 834000;
unsigned int fullUpdateCounter = 0;


void setup()
{
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

	
	

	#ifdef TASKSENDTOSCREEN
	xTaskCreate(
	TaskSendToScreen
	,  (const portCHAR *)"SendToScreen"   // A name just for humans
	,  1500  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,   2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	#endif

	#ifdef TASKCHECKSPC
	xTaskCreate(
	TaskCheckSPC
	,  (const portCHAR *)"CheckSPC"   // A name just for humans
	,  1000  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,   1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	#endif
	
	#ifdef TASKCHECKSERIALCOMMANDS
	xTaskCreate(
	TaskCheckSerialCommands
	,  (const portCHAR *)"CheckSerialCommands"   // A name just for humans
	,  2000  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	#endif

	#ifdef TASKGETPULLERRPM
	xTaskCreate(
	TaskGetPullerRPM
	,  (const portCHAR *)"GetPullerRPM"   // A name just for humans
	,  1000  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	#endif

	#ifdef TASKGETSPOOLRPM
	xTaskCreate(
	TaskGetSpoolRPM
	,  (const portCHAR *)"GetSpoolRPM"   // A name just for humans
	,  500  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	#endif

	#ifdef TASKUPDATETRAVERSE
	xTaskCreate(
	TaskUpdateTraverse
	,  (const portCHAR *)"UpdateTraverse"   // A name just for humans
	,  1000  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	#endif

	#ifdef TASKCHECKENCODER
	xTaskCreate(
	TaskCheckEncoder
	,  (const portCHAR *)"CheckEncoder"   // A name just for humans
	,  500  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	#endif
	
	#ifdef TASKGETFULLUPDATE
	xTaskCreate(
	TaskGetFullUpdate
	,  (const portCHAR *)"GetFullUpdate"   // A name just for humans
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
			//delay(10);
			//serialProcessing.CheckSerial(&Serial1, serialCommand.hardwareType);
			

			xSemaphoreGive(xSemaphore);
			vTaskDelayUntil( &xLastWakeTime, 50);
		}

		
	}

}

void TaskGetSpoolRPM(void *pvParameters)  // This is a task.
{
	while(1) // A Task shall never return or exit.
	{
		if ( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
		{
			TickType_t xLastWakeTime;
			xLastWakeTime = xTaskGetTickCount();

			SerialCommand serialCommand = {0};
			serialCommand.command = "SpoolRPM";
			serialCommand.hardwareType = hardwareType.traverse;
			serialCommand.value = NULL;
			
			if (!serialProcessing.FullUpdateRequested){
			serialProcessing.SendDataToDevice(&serialCommand);
			}
			
			xSemaphoreGive(xSemaphore);
			vTaskDelayUntil( &xLastWakeTime, 500);
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

			#ifdef TASKCHECKENCODER
			//SerialNative.println(screenEncoder.read());
			int32_t encoderValue = screenEncoder.read();
			if (previousEncoderValue != encoderValue)
			{
				
				SerialCommand sCommand = {0};
				sCommand.hardwareType = hardwareType.puller;

				//char decimalNumber[20] = {0};
				//CONVERT_FLOAT_TO_STRING(encoderValue, decimalNumber);
				
				
				if (encoderValue < previousEncoderValue)
				{
					sCommand.command = "increase_rpm";
				}
				if (encoderValue > previousEncoderValue)
				{
					sCommand.command = "decrease_rpm";
				}
				
				char value[MAX_CMD_LENGTH] = {0};
				CONVERT_NUMBER_TO_STRING(INT_FORMAT, abs(encoderValue - previousEncoderValue), value);
				
				sCommand.value = value;
				
				
				serialProcessing.SendDataToDevice(&sCommand);
				
				previousEncoderValue = encoderValue;
			}
			
			#endif

			xSemaphoreGive(xSemaphore);
			vTaskDelayUntil( &xLastWakeTime, 50);
		}

		
	}

}

void TaskGetFullUpdate(void *pvParameters)  // This is a task.
{
	while(1) // A Task shall never return or exit.
	{
		if ( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
		{
			TickType_t xLastWakeTime;
			xLastWakeTime = xTaskGetTickCount();
			if (serialProcessing.FullUpdateRequested)
			{
				SerialCommand command = {0};
				
				switch(fullUpdateCounter)
				{
					
					case 0:
						command.command = "velocity";
						command.hardwareType = hardwareType.puller;
						serialProcessing.SendDataToDevice(&command);
						fullUpdateCounter++;
						//vTaskDelay(50);
						break;
					case 1:
						command.command = "InnerOffset";
						command.hardwareType = hardwareType.traverse;
						serialProcessing.SendDataToDevice(&command);
						fullUpdateCounter++;
						//vTaskDelay(50);
						break;
					case 2:
						command.command = "SpoolWidth";
						command.hardwareType = hardwareType.traverse;
						serialProcessing.SendDataToDevice(&command);
						fullUpdateCounter++;
						//vTaskDelay(10);
						break;
					case 3:
						command.command = "RunMode";
						command.hardwareType = hardwareType.traverse;
						serialProcessing.SendDataToDevice(&command);
						fullUpdateCounter++;
						fullUpdateCounter = 0;
						serialProcessing.FullUpdateRequested = false;
						//vTaskDelay(10);
						break;
					default:
						fullUpdateCounter = 0;
						serialProcessing.FullUpdateRequested = false;
				}
				
			}
			
			xSemaphoreGive(xSemaphore);
			vTaskDelayUntil( &xLastWakeTime, 50);
		}

		
	}

}







