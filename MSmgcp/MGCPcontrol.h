#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "MGCPparser.h"
#include "EventAnn.h"
#include "EventCnf.h"

using namespace std;

extern SHP_IPar init_Params;
extern SHP_NETDATA net_Data;





class MGCPcontrol
{
public:
	MGCPcontrol();

	void Preprocessing(SHP_MGCP);
private:
	SHP_EventAnn annControl;
	SHP_EventCnf cnfControl;
};
typedef shared_ptr<MGCPcontrol> SHP_MGCPcontrol;