#pragma once
#include "../SharedSource/stdafx.h"
#include "../SharedSource/Structs.h"
#include "../SharedSource/Functions.h"
#include "MGCPparser.h"
#include "CallerBase.h"

extern SHP_STARTUP init_Params;
extern SHP_NETDATA net_Data;

class Cnf
{
public:
	Cnf(SHP_MGCP);

	void CRCX(SHP_MGCP, string, string);
	void MDCX(SHP_MGCP);
	string DLCX(SHP_MGCP);

	string CheckExistance(SHP_MGCP);
	bool DeleteCnf() { if (vecCallerBase.size() == 0) return true; else return false; }

	string eventNum = "";
private:
	SHP_CallerBase FindCallerBase(SHP_MGCP);
	void RemoveCallerBase(SHP_CallerBase);
	int ActivePoints();
	void SendToCnfModulCR();
	void SendToCnfModulMD_DL(SHP_CallerBase);
	
	bool state = false; 
	bool deleted = false;
	vector<SHP_CallerBase> vecCallerBase;
};
typedef shared_ptr<Cnf> SHP_Cnf;