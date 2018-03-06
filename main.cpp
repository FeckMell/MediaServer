#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "MGCPserver.h"



/*#ifdef WIN32
//#include "vld.h" //visual leak detector
#endif // WIN32*/

FILE *FileLog;
FILE *FileLogConfPoint;
FILE *FileLogConfRoom;
FILE *FileLogMixer;
FILE *FileLogMixerInit;
FILE *FileLogServer;
boost::gregorian::date Date;
string DateStr;
string PathEXE;

void GetPathExe(char* argv)
{
	boost::filesystem::path full_path(boost::filesystem::initial_path<boost::filesystem::path>());
	full_path = boost::filesystem::system_complete(boost::filesystem::path(argv));
	PathEXE = full_path.parent_path().string() + "\\";
}
int main(int argc, char* argv[])
{
	try
	{
		
		setlocale(LC_ALL, "Russian");
		cout << "\nВЕРСИЯ 1.0.1 (26.04.2016 / 11:42)";
		GetPathExe(argv[0]);
		OpenLogFiles();
		LogMain("Using path: " + PathEXE);
		/************************************************************************
			Парсинг входящих параметров	                                                                     
		************************************************************************/
		//string strConfigFile("mgcpserver.cfg");
		string strConfigFile(PathEXE + "mgcpserver.cfg");
		Config cfg;
		cfg = ParseConfig(strConfigFile, cfg);
		if (cfg.error == -1){ cout << "\nNO MEDIA " << cfg.error; system("pause"); return 0; }
		if (cfg.error == 0)
		{
			cout << "\nparse end:\nUsing IP: " << cfg.IP << " Media Path: " << cfg.MediaPath << " port: " << cfg.port;
			LogMain("\nparse end:\nUsing IP: " + cfg.IP + " Media Path: " + cfg.MediaPath + " port: " + boost::to_string(cfg.port));
		}
		//Инициализация адреса и порта сервера		
		boost::asio::io_service io_service;
		const udp::endpoint ep(boost::asio::ip::address::from_string(cfg.IP),cfg.port);
		/************************************************************************
			Запуск экземляра MGCP-сервера	                                                                     
		************************************************************************/
		CMGCPServer s({ io_service, ep, cfg.MediaPath });
		LogMain("Запуск экземляра MGCP-сервера	\n");
		s.Run();
		io_service.run();
	}
	catch (std::exception& e)
	{
		LogMain("Exception:" + boost::to_string(e.what()));
		cerr << "Exception: " << e.what() << "\n";
		MessBox("Exception:" + boost::to_string(e.what()));
		system("pause");
	}
	return 0;
}
