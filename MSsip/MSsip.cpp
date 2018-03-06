#include "stdafx.h"
#include "Server.h"


//*///------------------------------------------------------------------------------------------
//*///----Variables-----------------------------------------------------------------------------
SHP_STARTUP init_Params;
SHP_NETDATA net_Data;
BOOSTLOGGER lg;
string modul_name = "sip";
int modul_num = 4;
string version = "SIP v.2.0.1, build 10.11.2016 17:15";
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	try
	{
		/************************************************************************
		Инициализация
		************************************************************************/
		setlocale(LC_ALL, "Russian");//Russia location
		init_Params.reset(new STARTUP(argv, modul_name)); //Create storage for init parametrs and parse init file
		LogsInit(modul_name);
		BOOST_LOG_SEV(lg, fatal) << version << "\n" << init_Params->GetParams() << "\nIniting net services.";
		init_ffmpeg();
		net_Data.reset(new NETDATA(modul_num));
		/************************************************************************
		Запуск MGCP-сервера
		************************************************************************/
		BOOST_LOG_SEV(lg, debug) << "Initing done.->InnerServer server_inner;";
		SIPServer sip_server(modul_num, modul_name);
		BOOST_LOG_SEV(lg, debug) << "InnerServer server_inner;->server_inner.Run();";
		sip_server.Run();
		BOOST_LOG_SEV(lg, debug) << "server_inner.Run();->net_Data->GI(NETDATA::in).run();";
		net_Data->GI(NETDATA::in).run();
		BOOST_LOG_SEV(lg, debug) << "net_Data->GI(NETDATA::in).run();";
	}
	catch (std::exception& e)
	{
		BOOST_LOG_SEV(lg, fatal) << "exeption in main:" << e.what();
		system("pause");
	}
	return 0;
}