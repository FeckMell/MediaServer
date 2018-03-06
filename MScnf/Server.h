#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "InnerParser.h"
#include "Control.h"
using namespace std;
using boost::asio::ip::udp;

extern SHP_IPar init_Params;
//extern boost::asio::io_service io_Server;
extern boost::asio::io_service io_Apps;
//extern SHP_Socket outer_Socket;
extern SHP_Socket inner_Socket;

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class InnerServer
{
public:
	InnerServer();
	void Run();
	void Receive(boost::system::error_code, size_t);
private:
	void ReplyError(SHP_IPL);

	SHP_Control iplManagement;
	InnerMes message = (InnerMes());
};