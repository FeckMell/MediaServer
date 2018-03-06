#pragma once
#include "stdafx.h"
#include "SL_Config.h"


class LOG
{
public:

	static void Init();
	static BOOSTLOGGER GL(int);

	static vector<string> logNames;
	static BOOSTLOGGER vecLogs;

};