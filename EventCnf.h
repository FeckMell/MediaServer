#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "EventBase.h"
#include "MGCPparser.h"
#include "Conf.h"

using namespace std;

extern SHP_InitParams init_Params;
extern boost::asio::io_service io_Server;
extern boost::asio::io_service io_Apps;
extern SHP_Socket outer_Socket;
extern SHP_Socket inner_Socket;

class EventCnf :EventBase
{
public:
	void CRCX(SHP_MGCP);
	void MDCX(SHP_MGCP);
	void DLCX(SHP_MGCP);

private:
	string CheckExistance(SHP_MGCP);
	SHP_Conf FindConf(SHP_MGCP);
	void RemoveConf(SHP_Conf);

	vector<SHP_Conf> vecConf;
};
typedef shared_ptr<EventCnf> SHP_EventCnf;