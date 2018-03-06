#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "MGCPparser.h"
#include "CallerBase.h"

extern SHP_InitParams init_Params;

extern boost::asio::io_service io_Server;
extern boost::asio::io_service io_Apps;
extern SHP_Socket outer_Socket;
extern SHP_Socket inner_Socket;

class CallerAnn : public CallerBase
{
public:
	CallerAnn(SHP_MGCP, string, string);

	void RQNT(SHP_MGCP);
	void DLCX(SHP_MGCP);
	string eventNum="";
private:
	string filename = "";
	
};
typedef shared_ptr<CallerAnn> SHP_CallerAnn;