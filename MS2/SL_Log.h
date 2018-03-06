#pragma once
#include "stdafx.h"
#include "SL_Socket.h"

class LOG
{
public:

	static void Init();/*main, errors, fakesocket*/
	static void RegisterLogFile(string name_);

	static void Log(string where_, string level_, string what_);

private:

	static map<string, ofstream> _files;
	static SHP_thread _thread;
	static map<string, string> _data;/*lasdate*/

	static SHP_SOCKET _fake_socket;
	static void FakeReceive(boost::system::error_code, size_t);

};