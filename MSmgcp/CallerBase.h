#pragma once
#include "../SharedSource/stdafx.h"
#include "../SharedSource/Structs.h"
#include "../SharedSource/Functions.h"
#include "MGCPparser.h"

extern SHP_STARTUP init_Params;
extern SHP_NETDATA net_Data;

class CallerBase
{
public:
	CallerBase(SHP_MGCP, string, string);
	string serverSDP;
	string clientSDP;
	string serverPort;
	string clientPort;
	string serverIP;
	string clientIP;
	string callID;
	//string state = "off";//sendrecv, inactive, onhold, play, delete
	bool state = false;//true - sendrecv, false - inactive

	string ModifyCallerBase(SHP_MGCP);
protected:
private:
	string FindSDPmode(string);
	string ChangeSDPmode(string, string, string);
	string GetIPfromSDP(string);
	string GetPortFromSDP(string);
};
typedef shared_ptr<CallerBase> SHP_CallerBase;