#pragma once
#include "stdafx.h"
#include "SL_SockStorage.h"
#include "SL_BasicStructs.h"
#include "SL_Config.h"


class LOG
{
public:
	enum Lvl{ trace, debug, info, info2, error, fatal };
	static void Init();
	static void Log(int, string, string);
private:
	static void Write(int, string, string);

	static void ReinitFiles();
	static string TimeMS();
	static void CheckDate();

	static map<string, ofstream> files;
	static vector<string> fileNames;
	static SHP_SOCK socket;
	static string lastDate;

	static SHP_thread th;
	static void FakeReceive(boost::system::error_code, size_t);
	static uint8_t fake_data[10];
};