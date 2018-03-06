#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "Logger.h"
#include "MGCPserver.h"



/*#ifdef WIN32
//#include "vld.h" //visual leak detector
#endif // WIN32*/

boost::gregorian::date Date;
string DateStr;
string PathEXE;

Logger CLogger = Logger();
void Runner()
{
	CLogger.Run();
}
int main(int argc, char* argv[])
{
	try
	{
		setlocale(LC_ALL, "Russian");
		cout << "\nВЕРСИЯ 1.1.5 (11.05.2016 / 13:33)";
		GetPathExe(argv[0]);
		CLogger.Create();
		std::thread my_thread(&Runner);
		std::this_thread::sleep_for(std::chrono::microseconds(50));
		my_thread.detach();
		LogMain("Using path: " + PathEXE);
		/************************************************************************
			Парсинг входящих параметров	                                                                     
		************************************************************************/
		string strConfigFile(PathEXE + "mgcpserver.cfg");
		Config cfg;
		cfg = ParseConfig(strConfigFile, cfg);
		if (cfg.error == -1){ cout << "\nNO MEDIA " << cfg.error; system("pause"); return 0; }
		if (cfg.error == 0)
		{
			cout << "\nparse end:\nUsing IP: " << cfg.IP << " Media Path: " << cfg.MediaPath << " port: " << cfg.port;
			LogMain("\nparse end:\nUsing IP: " + cfg.IP + " Media Path: " + cfg.MediaPath + " port: " + std::to_string(cfg.port));
		}
		//Инициализация адреса и порта сервера		
		boost::asio::io_service io_service;
		const udp::endpoint ep(boost::asio::ip::address::from_string(cfg.IP),cfg.port);
		/************************************************************************
			Запуск экземляра MGCP-сервера	                                                                     
		************************************************************************/
		//cout << "\nthread MAIN " << std::this_thread::get_id();
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
