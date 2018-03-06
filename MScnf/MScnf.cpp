#include "stdafx.h"
#ifdef WIN32
#include "Mmsystem.h"
#endif
#include "Structs.h"
#include "Server.h"

using namespace std;

/*C:\Projects\LIBS\MGCPnew\Release\Announcements.exe "ann" "127.0.0.1" "2427" "10.77.7.5" "C:\Projects\LIBS\MGCPnew\Release\MediaFiles" "HOMEPATH" "100"*/
//*///------------------------------------------------------------------------------------------
//*///----Variables-----------------------------------------------------------------------------
SHP_IPar init_Params;
//boost::asio::io_service io_Server;
boost::asio::io_service io_Apps;
//SHP_Socket outer_Socket;
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
		//timeBeginPeriod(1);//CPU timer 1ms
		//SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);//Realtime priority in system
#endif
		setlocale(LC_ALL, "Russian");//Russia location
		cout << "\nТЕСТ ВЕРСИЯ 2.0 (20.10.2016 / 12:08)\n";//Info about app.
		/*if (argc != IPar::maxParamNames + 1)
		{
			cout << "\nNot enough init params! argc=" << argc;
			system("pause");
			exit(-1);
		}*///RELEASE
		init_Params.reset(new IPar(argv)); //Create storage for init parametrs and parse init file
		cout << init_Params->GetParams(); //show parametrs

		//Init sockets for modules and server
		inner_Socket.reset(new Socket(
			init_Params->data[IPar::innerIP],
			stoi(init_Params->data[IPar::innerPort]),
			io_Apps));
		//Init extra
		init_ffmpeg();

		/************************************************************************
		Запуск MGCP-сервера
		************************************************************************/
		InnerServer server_inner;
		server_inner.Run();
		io_Apps.run();

	}
	catch (std::exception& e)
	{
		cout << e.what();
		system("pause");
	}
	return 0;
}
