#pragma once
#include "../SharedSource/stdafx.h"
#include "../SharedSource/Structs.h"
#include "InnerParser.h"
#include "Control.h"
using namespace std;
using boost::asio::ip::udp;

extern SHP_IPar init_Params;
//extern string my_Modul_STR = "ANN";
//extern int my_Modul_INT = 2;

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class InnerServer
{
public:
	InnerServer(int, string);
	void Run();
	void Receive(boost::system::error_code, size_t);
private:
	void ReplyError(SHP_IPL);

	SHP_Control iplManagement;
	Request message = (Request());
};