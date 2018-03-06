#pragma once
#include "stdafx.h"
#include "Config.h"

//namespace LOG
class LOG
{
public:
	enum L{ main, mgcp, sip, ann, cnf, prx, dtmf, sql, maxL };

	static void Init();
	static BOOSTLOGGER GL(L);

	static vector<string> logNames;
	//static vector<BOOSTLOGGER> vecLogs;
	static BOOSTLOGGER vecLogs;
};