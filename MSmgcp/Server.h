#pragma once
#include "../SharedSource/stdafx.h"
#include "../SharedSource/Structs.h"
#include "MGCPparser.h"
#include "MGCPcontrol.h"

extern SHP_IPar init_Params;
extern SHP_NETDATA net_Data;

class MGCPServer
{
public:
	MGCPServer();
	void Run();
	void Receive(boost::system::error_code, size_t);
private:
	void ReplyError(SHP_MGCP);


	SHP_MGCPcontrol mgcpManagement;
	Request message = (Request());
};