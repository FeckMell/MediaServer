#pragma once
#include "../../stdafx.h"
#include "Config.h"
#include "Socket.h"


class LOG
{
public:

	static void Init();
	static void AddLogInstance(string file_);
	static void Log(string level_, string from_, string what_);

	static string Param(string name_);

private:

	static void Write(string level_, string from_, string what_);

	static void ReinitFiles();
	static string TimeMS();
	static void CheckDate();

	static map<string, ofstream> mapFile;
	static map<string, string> mapData;
	static map<string, int> mapLevel;

	

	static SHP_IO ioLog; 
	static SHP_Socket fakeSocket;
	static SHP_thread actionThread;
	static void FakeReceive(boost::system::error_code ec_, size_t size_);

};