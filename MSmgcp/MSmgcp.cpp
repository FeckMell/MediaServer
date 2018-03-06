#include "stdafx.h"
#include "Functions.h"
#include "Structs.h"
#include "Server.h"

//*///------------------------------------------------------------------------------------------
//*///----Variables-----------------------------------------------------------------------------
SHP_IPar init_Params;
SHP_NETDATA net_Data;
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
		init_Params.reset(new IPar(argv));
		LogsInit();
		BOOST_LOG_SEV(lg, fatal) << init_Params->GetParams()<<"\nIniting net services.";
		net_Data.reset(new NETDATA());
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
