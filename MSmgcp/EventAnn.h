#pragma once
#include "../SharedSource/stdafx.h"
#include "../SharedSource/Structs.h"
#include "../SharedSource/Functions.h"
#include "EventBase.h"
#include "MGCPparser.h"
#include "Ann.h"

extern SHP_STARTUP init_Params;
extern SHP_NETDATA net_Data;

class EventAnn :EventBase
{
public:
	void CRCX(SHP_MGCP);
	void RQNT(SHP_MGCP);
	void DLCX(SHP_MGCP);

private:
	string CheckExistance(SHP_MGCP);
	SHP_Ann FindAnn(SHP_MGCP);
	void RemoveAnn(SHP_Ann);

	vector<SHP_Ann> vecAnn;
};
typedef shared_ptr<EventAnn> SHP_EventAnn;
