/*
* SerialNative.cpp
*
* Created: 6/19/2019 3:10:25 PM
* Author: Anthony
*/


#include "SerialNative.h"
#include "board.h"
#include "DataConversions.h"

// default constructor
_SerialNative::_SerialNative()
{
} //SerialNative

//bool _SerialNative::begin(uint32_t baud_count)
//{
//SerialUSB.begin(baud_count);
//return false;
//}
//bool _SerialNative::available()
//{
//return SerialUSB.available();
//}

void _SerialNative::setTimeout(unsigned long timeout)
{
	//if (!USBD_Connected)
	//{
	//return;
	//}

	if (!dtr()){
		return;
	}

	if (SerialUSB) { SerialUSB.setTimeout(timeout);}
}

void _SerialNative::print(const char *c, ...)
{
	
	//if (!USBD_Connected)
	//{
	//return;
	//}

	if (!dtr()){
		return;
	}


	int ret=0;
	char vastr[MAX_CMD_LENGTH]={0};
	
	va_list ap;

	memset(vastr,0,MAX_CMD_LENGTH);
	va_start(ap,c);
	ret=vsprintf(vastr,(const char *)c,ap);
	va_end(ap);

	if (SerialUSB){	SerialUSB.print(vastr); }




}

void _SerialNative::print(char *c, ...)
{

	//if (!USBD_Connected)
	//{
	//return;
	//}

	if (!dtr()){
		return;
	}
	
	int ret=0;
	char vastr[MAX_CMD_LENGTH]={0};
	
	va_list ap;

	memset(vastr,0,MAX_CMD_LENGTH);
	va_start(ap,c);
	ret=vsprintf(vastr,(char *)c,ap);
	va_end(ap);

	if (SerialUSB){	SerialUSB.print(vastr); }




}

void _SerialNative::println(const char *c, ...)
{
	//if (!USBD_Connected)
	//{
		//return;
	//}

	if (!dtr()){
		return;
	}
	

	int ret=0;
	char vastr[MAX_CMD_LENGTH]={0};
	
	va_list ap;

	memset(vastr,0,MAX_CMD_LENGTH);
	va_start(ap,c);
	ret=vsprintf(vastr,(const char *)c,ap);
	va_end(ap);

	if (SerialUSB)
	{
		SerialUSB.println(vastr);
	}


	
}

void _SerialNative::println(char *c, ...)
{

	//if (!USBD_Connected)
	//{
	//return;
	//}

	if (!dtr()){
		return;
	}

	int ret=0;
	char vastr[MAX_CMD_LENGTH]={0};
	
	va_list ap;

	memset(vastr,0,MAX_CMD_LENGTH);
	va_start(ap,c);
	ret=vsprintf(vastr,(char *)c,ap);
	va_end(ap);

	if (SerialUSB){	SerialUSB.println(vastr); }
}

void _SerialNative::print(int num)
{

	//if (!USBD_Connected)
	//{
	//return;
	//}

	if (!dtr()){
		return;
	}

	if (SerialUSB){SerialUSB.print(num);}
}

void _SerialNative::println(int num)
{
	//if (!USBD_Connected)
	//{
	//return;
	//}

	if (!dtr()){
		return;
	}

	if (SerialUSB){SerialUSB.println(num);}
}

size_t _SerialNative::readBytesUntil(char terminator, uint8_t *buffer, size_t length)
{
	//if (!USBD_Connected)
	//{
		//return 0;
	//}

	if (!dtr()){
		return 0;
	}
	if (SerialUSB){ return readBytesUntil(terminator, (char *)buffer, length);}
}

size_t _SerialNative::readBytesUntil( char terminator, char *buffer, size_t length)
{
	//if (!USBD_Connected)
	//{
	//return;
	//}

	if (!dtr()){
		return 0;
	}
	if (SerialUSB){ return SerialUSB.readBytesUntil(terminator, buffer, length);}
}
int _SerialNative::read()
{
	//if (!USBD_Connected)
	//{
		//return 0;
	//}

	if (!dtr()){
		return 0;
	}
	if(SerialUSB) {return SerialUSB.read();}
}

void _SerialNative::ConcatenateChars(char *buffer_A, char *buffer_B, char *outputBuffer)
{
	
	sprintf(outputBuffer, SCREENFORMAT, buffer_A, buffer_B);
}

// default destructor
_SerialNative::~_SerialNative()
{
} //~SerialNative
