#include "stdafx.h"
#ifdef WIN32
#include "Mmsystem.h"
#endif
#include "Functions.h"
#include "Structs.h"
#include "Server.h"

//*///------------------------------------------------------------------------------------------
//*///----Variables-----------------------------------------------------------------------------
SHP_InitParams init_Params;

boost::asio::io_service io_Server;
boost::asio::io_service io_Apps;
SHP_Socket outer_Socket;
SHP_Socket inner_Socket;
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
		timeBeginPeriod(1);//CPU timer 1ms
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);//Realtime priority in system
#endif
		setlocale(LC_ALL, "Russian");//Russia location
		cout << "\nТЕСТ ВЕРСИЯ 2.0 (20.10.2016 / 12:08)\n";//Info about app.
		init_Params.reset(new InitParams(argv[0])); //Create storage for init parametrs and parse init file
		cout << init_Params->GetParams(); //show parametrs

		//Init sockets for modules and server
		outer_Socket.reset(new Socket(
			init_Params->data[InitParams::outerIP],
			stoi(init_Params->data[InitParams::outerPort]),
			io_Server));
		inner_Socket.reset(new Socket(
			init_Params->data[InitParams::innerIP],
			stoi(init_Params->data[InitParams::outerPort]),
			io_Server));
/************************************************************************
					 Запуск MGCP-сервера
************************************************************************/
		MGCPServer server_outer;
		server_outer.Run();
		io_Server.run();

	}
	catch (std::exception& e)
	{
		e;
	}
	return 0;
}
