#include "../SharedSource/stdafx.h"
#include "../SharedSource/Functions.h"
#include "../SharedSource/Structs.h"
#include "Server.h"

//*///------------------------------------------------------------------------------------------
//*///----Variables-----------------------------------------------------------------------------
SHP_STARTUP init_Params;
SHP_NETDATA net_Data;
BOOSTLOGGER lg;
string modulname = "mgcp";
int modulnum = 1;
string version = "mgcp v.2.0.1, build 10.11.2016 17:15";
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	try
	{
/************************************************************************
						Инициализация
************************************************************************/
		setlocale(LC_ALL, "Russian");
		init_Params.reset(new STARTUP(argv, modulname));
		LogsInit(modulname);
		BOOST_LOG_SEV(lg, fatal) << version << "\n" << init_Params->GetParams() << "\nIniting net services.";
		net_Data.reset(new NETDATA(modulnum));
/************************************************************************
					 Запуск MGCP-сервера
************************************************************************/
		BOOST_LOG_SEV(lg, debug) << "Initing done.->MGCPServer server_outer;";
		MGCPServer server_outer;
		BOOST_LOG_SEV(lg, debug) << "MGCPServer server_outer;->server_outer.Run();";
		server_outer.Run();
		BOOST_LOG_SEV(lg, debug) << "server_outer.Run();->net_Data->GI(NETDATA::out).run();";
		net_Data->GI(NETDATA::out).run();
		BOOST_LOG_SEV(lg, debug) << "net_Data->GI(NETDATA::out).run();";
	}
	catch (std::exception& e)
	{
		BOOST_LOG_SEV(lg, fatal) << "exeption in main:"<<e.what();
		system("pause");
	}
	return 0;
}
