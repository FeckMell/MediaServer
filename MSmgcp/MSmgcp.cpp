#include "../SharedSource/stdafx.h"
#include "../SharedSource/Functions.h"
#include "../SharedSource/Structs.h"
#include "Server.h"

//*///------------------------------------------------------------------------------------------
//*///----Variables-----------------------------------------------------------------------------
SHP_IPar init_Params;
SHP_NETDATA net_Data;
SHP_MGCPServer outer_Server;

string my_Modul_STR;
int my_Modul_INT;
src::severity_logger<severity_level> lg;
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
		my_Modul_STR = "MGCP";
		my_Modul_INT = 1;
		init_Params.reset(new IPar(argv, my_Modul_STR));
		LogsInit(my_Modul_STR);
		BOOST_LOG_SEV(lg, fatal) << init_Params->GetParams()<<"\nIniting net services.";
		net_Data.reset(new NETDATA(my_Modul_INT));
/************************************************************************
					 Запуск MGCP-сервера
************************************************************************/
		BOOST_LOG_SEV(lg, debug) << "Initing done.->MGCPServer server_outer;";
		//MGCPServer server_outer;
		outer_Server.reset(new MGCPServer());
		BOOST_LOG_SEV(lg, debug) << "MGCPServer server_outer;->server_outer.Run();";
		outer_Server->Run();
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
