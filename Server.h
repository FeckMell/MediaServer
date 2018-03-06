#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "MGCPparser.h"
#include "MGCPcontrol.h"
using namespace std;
using boost::asio::ip::udp;

extern SHP_InitParams init_Params;
extern boost::asio::io_service io_Server;
extern boost::asio::io_service io_Apps;
extern SHP_Socket outer_Socket;
extern SHP_Socket inner_Socket;

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class MGCPServer
{
public:
	MGCPServer();
	void Run();
	void Receive(boost::system::error_code, size_t);
private:
	void ReplyError(SHP_MGCP);


	SHP_MGCPcontrol mgcpManagement;
	OuterMes message = (OuterMes());
};