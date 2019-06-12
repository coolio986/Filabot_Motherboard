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
//#define TASKSERIALCOMMANDS
//#define TASKPULLER
//#define TASKTRAVERSE
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
void checkSPC();
// **** PROTOTYPES **** //



// These are used to get information about static SRAM and flash memory sizes
extern "C" char __data_start[];    // start of SRAM data
extern "C" char _end[];     // end of SRAM data (used to check amount of SRAM this program's variables use)
extern "C" char __data_load_end[];  // end of FLASH (used to check amount of Flash this program's code and data uses)

// declare since extern in board.h
bool SIMULATIONACTIVE = false; //sets default value for simulation


// ***** CLASS DECLARATIONs **** //
SerialCommand sCommand;
SpcProcessing spcProcessing;
Screen screen;
SerialProcessing serialProcessing;
SemaphoreHandle_t xSemaphore = NULL;
// ***** CLASS DECLARATIONs **** //



void setup()
{
	SerialUSB.begin(SERIAL_BAUD); //using native serial rather than programming port on DUE
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
	,  1000  // This stack size can be checked & adjusted by reading the Stack Highwater
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

	// ***** Instances ***** //

	vTaskStartScheduler(); //start FreeRTOS scheduler
	SerialUSB.println("Insufficient RAM"); //code execution should never get here, but could if there is not enough RAM
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


				
			

			

			//screen.UpdateSpool(); //TODO implement rpms for screen
			//screen.UpdatePuller();
			

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
			
			
			//if (spcProcessing.HasNewData)
			//{
				//SerialUSB.println(spcProcessing.GetDiameter()->charDiameterWithDecimal); //Serial print is broken using long values, use char instead
				//SerialCommand _serialCommand;
				//_serialCommand.hardwareType = hardwareType.screen;
				//_serialCommand.command = "Diameter";
				//_serialCommand.value = spcProcessing.GetDiameter()->charDiameterWithDecimal;
				//char output[MAX_CMD_LENGTH] = {0};
//
				//BuildSerialOutput(&_serialCommand, output);
				//Serial3.println(output);
			//}
			
			
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
			serialCommand.command = "getRpm";
			serialCommand.hardwareType = hardwareType.puller;
			
			serialProcessing.SendDataToDevice(&serialCommand);

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
			serialCommand.command = "getRpm";
			serialCommand.hardwareType = hardwareType.traverse;
			
			serialProcessing.SendDataToDevice(&serialCommand);

			xSemaphoreGive(xSemaphore);
			vTaskDelayUntil( &xLastWakeTime, 50);
		}

		
	}

}





