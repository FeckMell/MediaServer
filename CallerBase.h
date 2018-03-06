#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "MGCPparser.h"

extern SHP_InitParams init_Params;

extern boost::asio::io_service io_Server;
extern boost::asio::io_service io_Apps;
extern SHP_Socket outer_Socket;
extern SHP_Socket inner_Socket;

class CallerBase
{
public:
	CallerBase(SHP_MGCP, string, string);
	string server_SDP;
	string client_SDP;
	string server_Port;
	string client_Port;
	string server_IP;
	string client_IP;
	string callID;
	string state = "off";//off, ready,onhold,play, delete

	string ModifyCallerBase(SHP_MGCP);
protected:
private:
	string FindSDPmode(string);
	string ChangeSDPmode(string, string, string);
	string GetIPfromSDP(string);
	string GetPortFromSDP(string);
};
typedef shared_ptr<CallerBase> SHP_CallerBase;