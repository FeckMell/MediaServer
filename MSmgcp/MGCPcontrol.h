#pragma once
#include "../SharedSource/stdafx.h"
#include "../SharedSource/Structs.h"
#include "../SharedSource/Functions.h"
#include "EventAnn.h"
#include "EventCnf.h"
#include "MGCPparser.h"

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
	//*///------------------------------------------------------------------------------------------
	//*///------------------------------------------------------------------------------------------
	void ReplyClient(SHP_MGCP, string);
};
typedef shared_ptr<MGCPcontrol> SHP_MGCPcontrol;