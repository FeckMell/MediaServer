#pragma once
#include "stdafx.h"
#include "MGCPparser.h"

extern SHP_STARTUP init_Params;
extern SHP_NETDATA net_Data;

class EventBase
{
public:
	EventBase();
protected:
	/*Functionality*/
	string ReservePort();
	void FreePort(string);

	string ReserveEventID();
	void FreeEventID(string);

	string GenSDP(string, SHP_MGCP);

	/*Data*/
	static vector<int> usedPorts;
	static vector<int> usedEventID;
	static int lastSDP_ID;

	/*Statistics*/
	/*int numOfEventsTotal = 0;
	int numOfEventsNow = 0;

	int numOfCRCX = 0;
	int numOfDLCX = 0;
	int numOfMDCX = 0;
	int numOfRQNT = 0;

	int numOfREQUESTsBad = 0;
	int numofREQUESTsAll = 0;*/
};
typedef shared_ptr<EventBase> SHP_EventBase;