#pragma once
#include "../SharedSource/stdafx.h"
#include "../SharedSource/Structs.h"
#include "../SharedSource/Functions.h"
#include "MGCPparser.h"
#include "CallerBase.h"

extern SHP_IPar init_Params;
extern SHP_NETDATA net_Data;

class Ann : public CallerBase
{
public:
	Ann(SHP_MGCP, string, string);

	void RQNT(SHP_MGCP);
	void DLCX(SHP_MGCP);
	string eventNum = "";
private:
	bool CheckFileExistance(string);
	void SendToAnnModul(string);

	void ReplyClient(SHP_MGCP, string);
	void SendModul(int where_, string what_);

	string fileName = "";
	
};
typedef shared_ptr<Ann> SHP_Ann;