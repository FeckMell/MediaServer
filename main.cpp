#include "stdafx.h"
#ifdef WIN32
#include "Mmsystem.h"
#endif
#include "Structs.h"
#include "Functions.h"
#include "Logger.h"
#include "MGCPserver.h"
//#include "TimeAPI.h"

boost::gregorian::date Date;
string DateStr;
string PathEXE;
string MusicPath;
short int RTPport;

Logger* CLogger = new Logger();
void Runner()
{
	CLogger->Run();
}
int main(int argc, char* argv[])
{
	try
	{
#ifdef WIN32
		timeBeginPeriod(1);
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
#endif
		DateStr = GetDate();
		setlocale(LC_ALL, "Russian");
		cout << "\nТЕСТ ВЕРСИЯ 1.50 (12.09.2016 / 12:08)";
		GetPathExe(argv[0]);
#ifdef __linux__
		PathEXE.pop_back();
		PathEXE.pop_back();
		cout<<"\nPATH="<<PathEXE<<"\n";
#endif
		CLogger->Create();
		boost::thread my_thread(&Runner);
		my_thread.detach();
		std::this_thread::sleep_for(std::chrono::microseconds(50));

		LogMain("Using path: " + PathEXE);
		/************************************************************************
			Парсинг входящих параметров
		************************************************************************/
		string strConfigFile(PathEXE + "mgcpserver.cfg");
		Config cfg;
		cfg = ParseConfig(strConfigFile, cfg);
		RTPport = cfg.RTPport;
#ifdef __linux__
		cfg.MediaPath = PathEXE+cfg.MediaPath;
#endif
		if (cfg.error == -1){ cout << "\nNO MEDIA " << cfg.error; system("pause"); return 0; }
		if (cfg.error == 0)
		{
			cout << "\nparse end:\nUsing IP: " << cfg.IP << "\nMedia Path: " << cfg.MediaPath << "\nMGCPport: " << cfg.MGCPport << "\nRTPPort: " << cfg.RTPport;
			LogMain("\nparse end:\nUsing IP: " + cfg.IP + "\nMedia Path: " + cfg.MediaPath + "\nMGCPport: " + boost::to_string(cfg.MGCPport) + "\nRTPPort: " + boost::to_string(cfg.RTPport));
		}
		//Инициализация адреса и порта сервера
		boost::asio::io_service io_service;
		const udp::endpoint MGCPep(boost::asio::ip::address::from_string(cfg.IP),cfg.MGCPport);
		///////////////////////////////////////////////////////////////////////////
		av_log_set_level(0);
		av_register_all();
		avcodec_register_all();
		avfilter_register_all();
		avformat_network_init();
		/************************************************************************
			Запуск экземляра MGCP-сервера
		************************************************************************/
		CMGCPServer s({ io_service, MGCPep, cfg.MediaPath });
		LogMain("Запуск экземляра MGCP-сервера");
		cout << "\n-------------------------------------------------------------------------------\n\n";
		s.Run();
		s.RunBuffer();
		io_service.run();
	}
	catch (std::exception& e)
	{
		LogMain("Exception:1" + boost::to_string(e.what()));
		cerr << "Exception:2 " << e.what() << "\n";
#ifdef WIN32
		MessBox("Exception:3" + boost::to_string(e.what()));
#endif
		system("pause");
	}
	return 0;
}
