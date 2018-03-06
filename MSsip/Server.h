#pragma once
#include "stdafx.h"
#include "SIPparser.h"
#include "SIPcontrol.h"

extern SHP_STARTUP init_Params;
extern SHP_NETDATA net_Data;

class SIPServer
{
public:
	SIPServer(int num_, string name_);
	void Run();
	void ReceiveSIP(boost::system::error_code, size_t);
	void ReceiveIN(boost::system::error_code, size_t);
private:
	//SHP_IPLcontrol iplManagement;
	SHP_SIPcontrol sipManagement;
	REQUEST message = (REQUEST());
};