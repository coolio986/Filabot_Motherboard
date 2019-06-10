/*===========================================================
* Project: Digital_Indicator_Firmware
* Developled using Arduino v1.8.5
* Developed by: Anthony Kaul(3D Excellence LLC) in collaboration with Filabot (Triex LLC)
* This firmware converts SPC (Statistical Process Control) into signals that can be recognized by a PC
* All rights reserved


* Version information:
* v1.2 - beta
* ===========================================================*/

#include <Arduino.h>
//#include <Arduino_FreeRTOS.h>
//#include <semphr.h>  // add the FreeRTOS functions for Semaphores (or Flags).

#include "SerialProcessing.h"
#include "hardwareTypes.h"
#include "SpcProcessing.h"
#include "Screen.h"
#include "Device_Configuration.h"
#include "Error.h"
#include "DataConversions.h"

#include <FreeRTOS_ARM.h>
// Redefine AVR Flash string macro as nop for ARM
#undef F
#define F(str) str



#define INCLUDE_vTaskDelay   1
#define configUSE_PREEMPTION 1

//#define TASKSCREEN
#define TASKSPC
//#define TASKSERIALCOMMANDS
//#define TASKPULLER
//#define TASKTRAVERSE


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
void checkSPC ();




// These are used to get information about static SRAM and flash memory sizes
extern "C" char __data_start[];    // start of SRAM data
extern "C" char _end[];     // end of SRAM data (used to check amount of SRAM this program's variables use)
extern "C" char __data_load_end[];  // end of FLASH (used to check amount of Flash this program's code and data uses)


bool SIMULATIONACTIVE = false; //sets default value for simulation


SerialCommand sCommand;
SpcProcessing spcProcessing;
Screen screen;
SerialProcessing serialProcessing;


SemaphoreHandle_t xSemaphore = NULL;



void setup()
{
	SerialUSB.begin(SERIAL_BAUD);
	//Serial.begin(SERIAL_BAUD);
	Serial3.begin(SERIAL_BAUD);
	//Serial.println("starting");

	spcProcessing.init();
	screen.init();
	serialProcessing.init();
	startErrorHandler();


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

	vTaskStartScheduler();
	SerialUSB.println("Insufficient RAM");
	while(1);
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
				screen.SendError(spcProcessing.GetError());
			}
			else
			{
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
			
			while (!spcProcessing.ISR_READY())
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
			
			
			spcProcessing.RunSPCDataLoop();
			spcProcessing.StopQuery();
			
			
			//SerialUSB.println()

			if (spcProcessing.newData)
			{
				SerialUSB.println(spcProcessing.GetDiameter()->charDiameterWithDecimal);
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





