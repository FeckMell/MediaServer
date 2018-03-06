#include "../SharedSource/stdafx.h"
#include "../SharedSource/Structs.h"
#include "../SharedSource/Functions.h"
#include "Server.h"

using namespace std;

//*///------------------------------------------------------------------------------------------
//*///----Variables-----------------------------------------------------------------------------
SHP_IPar init_Params;
SHP_NETDATA net_Data;

string my_Modul_STR = "CNF";
int my_Modul_INT = 3;

src::severity_logger<severity_level> lg;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	try
	{
/************************************************************************
				�������������
************************************************************************/
		setlocale(LC_ALL, "Russian");//Russia location
		init_Params.reset(new IPar(argv, my_Modul_STR)); //Create storage for init parametrs and parse init file
		LogsInit(my_Modul_STR);
		BOOST_LOG_SEV(lg, fatal) << init_Params->GetParams() << "\nIniting net services.";
		init_ffmpeg();
		net_Data.reset(new NETDATA(my_Modul_INT));
/************************************************************************
			������ MGCP-�������
************************************************************************/
		BOOST_LOG_SEV(lg, debug) << "Initing done.->InnerServer server_inner;";
		InnerServer server_inner;// (/*my_Modul_INT, my_Modul_STR*/);
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