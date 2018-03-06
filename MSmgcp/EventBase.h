#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "MGCPparser.h"

extern SHP_IPar init_Params;
extern SHP_NETDATA net_Data;





class EventBase
{
public:
	EventBase();
protected:
	/*Functionality*/
	string ReservePort();
	void FreePort(string);

	string ReserveEventNum();
	void FreeEventNum(string);

	string GenSDP(string, SHP_MGCP);

	/*Data*/
	static vector<int> usedPorts;
	static vector<int> usedEventNum;
	static int lastSDP_ID;

	/*Statistics*/
	/*int numOfEventsTotal = 0;
	int numOfEventsNow = 0;

	int numOfCRCX = 0;
	int numOfDLCX = 0;
	int numOfMDCX = 0;
	int numOfRQNT = 0;

	int numOfRequestsBad = 0;
	int numofRequestsAll = 0;*/
};
typedef shared_ptr<EventBase> SHP_EventBase;