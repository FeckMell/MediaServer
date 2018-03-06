#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "EventBase.h"
#include "MGCPparser.h"
#include "Cnf.h"

using namespace std;

extern SHP_IPar init_Params;
extern SHP_NETDATA net_Data;





class EventCnf :EventBase
{
public:
	void CRCX(SHP_MGCP);
	void MDCX(SHP_MGCP);
	void DLCX(SHP_MGCP);

private:
	string CheckExistance(SHP_MGCP);
	SHP_Cnf FindCnf(SHP_MGCP);
	void RemoveCnf(SHP_Cnf);

	vector<SHP_Cnf> vecCnf;
};
typedef shared_ptr<EventCnf> SHP_EventCnf;