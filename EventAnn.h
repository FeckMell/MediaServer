#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "EventBase.h"
#include "MGCPparser.h"
#include "CallerAnn.h"

using namespace std;

extern SHP_InitParams init_Params;
extern boost::asio::io_service io_Server;
extern boost::asio::io_service io_Apps;
extern SHP_Socket outer_Socket;
extern SHP_Socket inner_Socket;

class EventAnn :EventBase
{
public:
	void CRCX(SHP_MGCP);
	void RQNT(SHP_MGCP);
	void DLCX(SHP_MGCP);

private:
	string CheckExistance(SHP_MGCP);
	SHP_CallerAnn FindAnn(SHP_MGCP);
	void RemoveAnn(SHP_CallerAnn);

	vector<SHP_CallerAnn> vecAnn;
};
typedef shared_ptr<EventAnn> SHP_EventAnn;
