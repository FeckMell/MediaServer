#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "Server.h"

using namespace std;

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
#ifdef WIN32
		timeBeginPeriod(1);//CPU timer 1ms
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);//Realtime priority in system
#endif
		setlocale(LC_ALL, "Russian");//Russia location
		init_Params.reset(new IPar(argv)); //Create storage for init parametrs and parse init file
		LogsInit();
		BOOST_LOG_SEV(lg, fatal) << init_Params->GetParams() << "\nIniting net services.";
		init_ffmpeg();
		net_Data.reset(new NETDATA());
/************************************************************************
			Запуск MGCP-сервера
************************************************************************/
		BOOST_LOG_SEV(lg, debug) << "Initing done.->InnerServer server_inner;";
		InnerServer server_inner;
		BOOST_LOG_SEV(lg, debug) << "InnerServer server_inner;->server_inner.Run();";
		server_inner.Run();
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
