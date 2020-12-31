#pragma once

#ifndef DATATRANSIT_H
#define DATATRANSIT_H

#define WIN32_LEAN_AND_MEAN   
#include <webxlib.h>

class DataTransit;

typedef struct TransitEvent
{
	DataTransit* _transit;
	void* _callbackRoutinePtr;
} TransitCallback;

class DataTransit
{
public:
	DataTransit(csockdata* _config);
	~DataTransit();

	csockdata* GetTransitConfiguration();
	bool IsOnline();

	webxlib::csocket* GetTransitListener();
	webxlib::csocket* GetConnectedSocket();

	void EstablishTransit();
	void StopTransit();

	int WriteData(const char* data, int size);
	int WriteData(const char* data, int size, TransitCallback* _preSend, TransitCallback* _postSend);

	int RecvData(char* data, int size);
	int RecvData(char* data, int size, TransitCallback* _preRecv, TransitCallback* _postRecv);
protected:
	webxlib::csocket* _listener = nullptr;
	webxlib::csocket* _consock = nullptr;

	csockdata* _transitConfig = nullptr;

	bool _transitOnline = false;

	static DWORD WINAPI _processReceivedPackage(LPVOID arg);

	static void _catalyst(void* arg);
};

#endif //DATATRANSIT_H