#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "MGCPparser.h"
#include "CallerBase.h"

using namespace std;

extern SHP_InitParams init_Params;
extern boost::asio::io_service io_Server;
extern boost::asio::io_service io_Apps;
extern SHP_Socket outer_Socket;
extern SHP_Socket inner_Socket;

class Conf
{
public:
	Conf(SHP_MGCP);

	void CRCX(SHP_MGCP, string, string);
	void MDCX(SHP_MGCP);
	string DLCX(SHP_MGCP);

	string CheckExistance(SHP_MGCP);
	bool DeleteConf() { if (vecCallerBase.size() == 0) return true; else return false; }

	string eventNum = "";
private:
	
	SHP_CallerBase FindCallerBase(SHP_MGCP);
	void RemoveCallerBase(SHP_CallerBase);

	vector<SHP_CallerBase> vecCallerBase;
};
typedef shared_ptr<Conf> SHP_Conf;