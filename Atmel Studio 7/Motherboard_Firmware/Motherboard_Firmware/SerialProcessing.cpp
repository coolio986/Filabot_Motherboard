#include "SerialProcessing.h"
#include <Arduino.h>
#include "board.h"
#include "hardwareTypes.h"
#include <stdio.h>
#include "DataConversions.h"
#include "Structs.h"
#include "SerialPortExpander.h"
#include "SerialNative.h"



template<size_t SIZE, class T> inline size_t array_size(T (&arr)[SIZE]);

SerialProcessing *SerialProcessing::firstInstance;
SerialPortExpander _serialPortExpander;

//bool SIMULATIONMODE; //sets default value for simulation


SerialProcessing::SerialProcessing(){
	if(!firstInstance){
		firstInstance = this;
	}
}

void SerialProcessing::init(){
	_serialPortExpander.init();
	SerialNative.setTimeout(1);

}

void SerialProcessing::Poll(void)
{

	if (!commandActive)
	{
		CheckSerial(&SerialNative, 0); //check for new data coming from PC
		CheckSerial(&Serial1, 1); //check for new data coming from expander
	}

}

unsigned int SerialProcessing::CheckSerial(Stream *port, int portNumber) //experimental
{
	commandActive = true;
	char computerdata[MAX_CMD_LENGTH] = {0};
	byte computer_bytes_received = 0;

	SerialCommand sCommand;
	sCommand.command = NULL;
	sCommand.hardwareType = NULL;
	sCommand.value = NULL;
	port->setTimeout(1);

	int pos = 0;
	long start = millis();
	//while (port->available() >= MAX_CMD_LENGTH)
	//{

	
	
	//computerdata[pos] = port->read();
	//
	//
	//if (computerdata[pos] == 10 || computerdata[pos] == 13 || pos >= MAX_CMD_LENGTH || computerdata[pos] == 0)
	//{
	//computerdata[pos] = 0;
	//break;
	//}
	//
	//
	//pos++;
	//
	//if ((millis() - start) > 1000)
	//{
	//break;
	//}
	//}
	//
	//if (pos > 0)
	//{
	//long duration  = millis() - start;
	//
	//int test = 0;
	//}

	if (port->available() > 0)
	{
		for(int i = 0; i < MAX_CMD_LENGTH; i++)
		{
			computerdata[i] = 0; // clear the array
		}

		port->readBlock(computerdata, MAX_CMD_LENGTH);
		
		//computer_bytes_received = port->findUntil()
		//port->readBytes(computerdata, MAX_CMD_LENGTH);
		
		for (int i = 0; i < MAX_CMD_LENGTH; i++){
			if (computerdata[i] > 0) {
				if (computerdata[i] == 10 || computerdata[i] == 13){
					computerdata[i] = 0;
					break;
				}
				
				computer_bytes_received++;
			}
		}
		//computer_bytes_received = port->readBytesUntil(10, computerdata, MAX_CMD_LENGTH); //We read the data sent from the serial monitor(pc/mac/other) until we see a <CR>. We also count how many characters have been received
		//computerdata[computer_bytes_received] = 0; //We add a 0 to the spot in the array just after the last character we received.. This will stop us from transmitting incorrect data that may have been left in the buffer
		
		//while(port->available()){
		//SerialNative.println(port->read());
		//} //flush buffer
	}
	
	if (computer_bytes_received > 0){
		long duration  = millis() - start;
		int test = 0;
	}


	if (computer_bytes_received != 0) {             //If computer_bytes_received does not equal zero
		
		CommandParse(&sCommand, computerdata);
		
		computer_bytes_received = 0;                  //Reset the var computer_bytes_received to equal 0
		
		if (portNumber == 0) //if data comes from USB/PC
		{
			ProcessDataFromPC(&sCommand);
		}
		else //if data comes from expander
		{
			SendToPC(&sCommand);
			SendScreenData(&sCommand);
		}
		
	}
	commandActive = false;

	return 1;
}

unsigned int SerialProcessing::CheckSerial(HardwareSerial *port, int portNumber)
{

	commandActive = true;
	char computerdata[MAX_CMD_LENGTH] = {0};
	byte computer_bytes_received = 0;

	SerialCommand sCommand;
	sCommand.command = NULL;
	sCommand.hardwareType = NULL;
	sCommand.value = NULL;

	if (port->available() > 0)
	{
		for(int i = 0; i < MAX_CMD_LENGTH; i++)
		{
			computerdata[i] = 0; // clear the array
		}

		computer_bytes_received = port->readBytesUntil(10, computerdata, MAX_CMD_LENGTH); //We read the data sent from the serial monitor(pc/mac/other) until we see a <CR>. We also count how many characters have been received
		computerdata[computer_bytes_received] = 0; //We add a 0 to the spot in the array just after the last character we received.. This will stop us from transmitting incorrect data that may have been left in the buffer
		
		while(port->available()){
			SerialNative.println(port->read());
		} //flush buffer
	}

	if (computer_bytes_received != 0) {             //If computer_bytes_received does not equal zero
		
		CommandParse(&sCommand, computerdata);
		
		computer_bytes_received = 0;                  //Reset the var computer_bytes_received to equal 0
		
		if (portNumber == 0) //if data comes from USB/PC
		{
			ProcessDataFromPC(&sCommand);
		}
		else //if data comes from expander
		{
			SendToPC(&sCommand);
			SendScreenData(&sCommand);
		}
		
	}
	commandActive = false;

	return 1;
}

//unsigned int SerialProcessing::CheckSerial(_SerialNative *port, int portNumber)
//{
//
//commandActive = true;
//char computerdata[MAX_CMD_LENGTH] = {0};
//byte computer_bytes_received = 0;
//
//SerialCommand sCommand;
//sCommand.command = NULL;
//sCommand.hardwareType = NULL;
//sCommand.value = NULL;
//
//if (port->available() > 0)
//{
//for(int i = 0; i < MAX_CMD_LENGTH; i++)
//{
//computerdata[i] = 0; // clear the array
//}
//
//computer_bytes_received = port->readBytesUntil(10, computerdata, MAX_CMD_LENGTH); //We read the data sent from the serial monitor(pc/mac/other) until we see a <NL>. We also count how many characters have been received
//computerdata[computer_bytes_received] = 0; //We add a 0 to the spot in the array just after the last character we received.. This will stop us from transmitting incorrect data that may have been left in the buffer
//
//while(port->available()){
//SerialNative.println(port->read());
//} //flush buffer
//}
//
//if (computer_bytes_received != 0) {             //If computer_bytes_received does not equal zero
//
//CommandParse(&sCommand, computerdata);
//
//computer_bytes_received = 0;                  //Reset the var computer_bytes_received to equal 0
//
//if (portNumber == 0) //if data comes from USB/PC
//{
//ProcessDataFromPC(&sCommand);
//}
//else //if data comes from expander
//{
//SendToPC(&sCommand);
//SendScreenData(&sCommand);
//}
//
//}
//commandActive = false;
//
//return 1;
//}

unsigned int SerialProcessing::CommandParse(SerialCommand *sCommand, char str[MAX_CMD_LENGTH])
{

	char *hardwareID = strtok(str, DELIMITER); //hardware ID
	char *cmd = strtok(NULL, DELIMITER);
	char *arguments = strtok(NULL, DELIMITER);

	for (int i=0; hardwareID[i]!= '\0'; i++)
	{
		//Serial.println(hardwareType[i]);
		if (!isdigit(hardwareID[i]) != 0)
		{
			SerialNative.println("Invalid Hardware ID, number is not a digit");
			return 0;
		}
	}

	sCommand->hardwareType = atoi(hardwareID);
	sCommand->command = cmd;
	sCommand->value = arguments;


	
	return 1;
}

unsigned int SerialProcessing::ProcessDataFromPC(SerialCommand *sCommand)
{
	if(sCommand->hardwareType == hardwareType.internal)
	{
		this->CheckInteralCommands(sCommand);
	}

	this->SendDataToDevice(sCommand);

	//if((sCommand->hardwareType > hardwareType.indicator) && (sCommand->hardwareType < hardwareType.screen))
	//{
	////serialPortExpander.channel
	//serialPortExpander.ProcessSerialExpander(sCommand);
	//}

	

	return 1;
}

unsigned int SerialProcessing::SendDataToDevice(SerialCommand *sCommand)
{

	if((sCommand->hardwareType > hardwareType.indicator) && (sCommand->hardwareType < hardwareType.screen))
	{
		//serialPortExpander.channel
		_serialPortExpander.ProcessSerialExpander(sCommand);
	}

	return 1;
}

unsigned int SerialProcessing::SendToPC(SerialCommand *sCommand)
{
	char serialOutputBuffer[MAX_CMD_LENGTH] = {0};
	BuildSerialOutput(sCommand, serialOutputBuffer);
	SerialNative.println(serialOutputBuffer);

	return 1;

}

unsigned int SerialProcessing::SendScreenData(SerialCommand *sCommand)
{

	if (sCommand->hardwareType == hardwareType.traverse || sCommand->hardwareType == hardwareType.screen)
	{
		SerialCommand _sCommand;
		_sCommand.hardwareType = hardwareType.screen;
		_sCommand.value = sCommand->value;
		_sCommand.command = sCommand->command;
		char serialOutputBuffer[MAX_CMD_LENGTH] = {0};
		BuildSerialOutput(&_sCommand, serialOutputBuffer);
		Serial3.println(serialOutputBuffer);
		//SerialUSB.println(serialOutputBuffer); //Serial print is broken using long values, use char instead

	}
	return 1;
}

void SerialProcessing::CheckInteralCommands(SerialCommand *sCommand)
{
	//Serial.println("sim mode routine");
	if (strcmp(sCommand->command, "IsInSimulationMode") == 0)
	{

		SIMULATIONACTIVE = strcmp(sCommand->value, "true") == 0;
	}


}











//void SerialProcessing::CheckSerialCommands()
//{
//if (Serial.available() > 0 )
//{
//SerialCommand sCommand;
////char *usbData = CheckSerial(&Serial);
//CheckSerial(&Serial);
////sCommand = GetSerialArgs(usbData);
//
//
//if(sCommand.hardwareType == hardwareType.internal)
//{
//this->CheckInteralCommands(&sCommand);
//}
//
//if(sCommand.hardwareType == hardwareType.spooler)
//{
//this->CheckSpoolerCommands(&sCommand);
//}
//
//if((sCommand.hardwareType > hardwareType.indicator) && (sCommand.hardwareType < hardwareType.internal))
//{
////serialPortExpander.channel
//serialPortExpander.ProcessSerialExpander(&sCommand);
//}
//
//
//
//
//
//
////if (!SIMULATIONACTIVE)
////{
////
////char * serialData = CheckSerial(&Serial);
////if (strlen(serialData) > 1)
////{
////Serial.print(sCommand.hardwareType);
////Serial.print(";");
////Serial.print(sCommand.command);
////Serial.println(";");
////Serial.println(sCommand.value);
////}
////}
//}
//
//if (Serial1.available() > 0)
//{
//
//}
//}



//void SerialProcessing::CheckSpoolerCommands(SerialCommand *sCommand)
//{
//
//if (SIMULATIONACTIVE){
//if (startsWith("getrpm", sCommand->command))
//{
//PrintRandomRPMData();
//}
//}
//else
//{
////Serial.print(sCommand->hardwareType);
////Serial.print(";");
////Serial.print(sCommand->command);
////Serial.println(";");
////Serial.println(sCommand->value);
//}
//
//}






//int SerialProcessing::strcicmp(char const *a, char const *b)
//{
//for (;; a++, b++) {
//int d = tolower(*a) - tolower(*b);
//if (d != 0 || !*a)
//return d;
//}
//}

//char *SerialProcessing::GetSerialOutputBuffer(void)
//{
//newData = false;
//return serialOutputBuffer;
//}


//char * CleaneData(char *data)
//{
//char endMarker = 0x00; //null character
//boolean newData = false;
//char *chars2 = (char*)malloc(numChars);
//char chars[numChars] = {};
//
//for (byte i = 0; i < numChars; i++)
//{
//if (data[i] != endMarker && newData == false )
//{
//
//if (data[i] != 0x0A)
//{
//chars[i] = data[i];
//}
//else
//{
//chars[i] = 0x20;
//}
//}
//
//else
//{
//i = 0;
//chars2 = chars;
////Serial.println(chars2);
//newData = true;
//return chars2;
//}
//}
//
//return chars2;
//}




//SerialCommand GetSerialArgs(char *serialData)
//{
//SerialCommand c;
//char delim[] = ";";
//char *ptr = strtok(serialData, delim);
//uint16_t loopCounter = 0;
//char *splitStrings[10] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
//
//while(ptr != NULL)
//{
//if (loopCounter <= 10){ splitStrings[loopCounter] = ptr;}
//ptr = strtok(NULL, delim);
//loopCounter++;
//}
////for (int i = loopCounter; i < 10; i++)
////{
////splitStrings[i] = NULL; //null out rest of point array
////}
//
//if (!ExistsInIntArray(int_hardwareTypes, array_size(int_hardwareTypes), atoi(splitStrings[0]))){return c;}
//
//
//c.hardwareType = atoi(splitStrings[0]);
//c.command = splitStrings[1];
//c.value = splitStrings[2];
//
//return c;
//
//}

//bool ExistsInIntArray(uint16_t *arrayToCheck, size_t arraySize, uint16_t numberToCheck)
//{
//
//
//for (unsigned int i = 0; i < arraySize; i++)
//{
//if (arrayToCheck[i] == numberToCheck)
//{
//return true;
//}
//}
//return false;
//}

void BuildSerialOutput(SerialCommand *sCommand, char *outputBuffer)
{
	if (sCommand->value == NULL)
	{
		sprintf(outputBuffer, OUTPUT_STRING_DS, sCommand->hardwareType, sCommand->command, sCommand->value);
		return;
	}

	sprintf(outputBuffer, OUTPUT_STRING_DSS, sCommand->hardwareType, sCommand->command, sCommand->value);
}

//template<size_t SIZE, class T> inline size_t array_size(T (&arr)[SIZE]) {
//return SIZE;
//}

void PrintRandomRPMData()
{
	long rpm = random(10, 15);

	//SerialNative.print(hardwareType.puller);
	//SerialNative.print(";getrpm = ");
	//SerialNative.print(rpm);
	//SerialNative.println(";");
}

//bool startsWith(const char *pre, const char *str)
//{
//size_t lenpre = strlen(pre),
//lenstr = strlen(str);
//return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
//}

// default destructor
SerialProcessing::~SerialProcessing()
{
} //~SerialProcessing

