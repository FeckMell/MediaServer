#include "stdafx.h"
#ifdef WIN32
#include "Mmsystem.h"
#endif
#include "Functions.h"
#include "Structs.h"
#include "Server.h"

//*///------------------------------------------------------------------------------------------
//*///----Variables-----------------------------------------------------------------------------
SHP_IPar init_Params;
SHP_NETDATA net_Data;
//boost::asio::io_service io_Server;
//boost::asio::io_service io_Apps;
//SHP_Socket outer_Socket;
//SHP_Socket inner_Socket;
//udp::endpoint inner_EndPoint;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	try
	{
/************************************************************************
						Инициализация
************************************************************************/
#ifdef WIN32
		//timeBeginPeriod(1);//CPU timer 1ms
		//SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);//Realtime priority in system
#endif
		setlocale(LC_ALL, "Russian");//Russia location
		cout << "\nТЕСТ MSmgcp 2.0 (20.10.2016 / 12:08)\n";//Info about app.
		init_Params.reset(new IPar(argv[0])); //Create storage for init parametrs and parse init file
		cout << init_Params->GetParams(); //show parametrs
		net_Data.reset(new NETDATA());
		//Init sockets for modules and server
		/*outer_Socket.reset(new Socket(
			init_Params->data[IPar::outerIP],
			stoi(init_Params->data[IPar::outerPort]),
			io_Server));
		inner_Socket.reset(new Socket(
			init_Params->data[IPar::innerIP],
			stoi(init_Params->data[IPar::outerPort]),
			io_Server));
		inner_EndPoint = udp::endpoint(
			boost::asio::ip::address::from_string("127.0.0.1"),
			stoi("2427")
			);*/
/************************************************************************
					 Запуск MGCP-сервера
************************************************************************/
		MGCPServer server_outer;
		server_outer.Run();
		net_Data->GI(NETDATA::out).run();
		//io_Server.run();

	}
	catch (std::exception& e)
	{
		cout<<e.what();
		system("pause");
	}
	return 0;
}
