#include "stdafx.h"
#include "Functions.h"
#include "Structs.h"
#include "Logger.h"
#include "MGCPserver.h"
#include "Mmsystem.h"
//#include "TimeAPI.h"



/*#ifdef WIN32
//#include "vld.h" //visual leak detector
#endif // WIN32*/
boost::gregorian::date Date;
string DateStr;
string PathEXE;
string MusicPath;
short int RTPport;

Logger CLogger = Logger();
void Runner()
{
	CLogger.Run();
}
int main(int argc, char* argv[])
{
	try
	{
		timeBeginPeriod(1);
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		DateStr = GetDate();
		setlocale(LC_ALL, "Russian");
		cout << "\nВЕРСИЯ 1.5.1 (11.07.2016 / 11:34)";
		GetPathExe(argv[0]);
		CLogger.Create();
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
		MusicPath = cfg.MediaPath;
		RTPport = cfg.RTPport;
		if (cfg.error == -1){ cout << "\nNO MEDIA " << cfg.error; system("pause"); return 0; }
		if (cfg.error == 0)
		{
			cout << "\nparse end:\nUsing IP: " << cfg.IP << " Media Path: " << cfg.MediaPath << " port: " << cfg.port << " RTPPort: " + boost::to_string(cfg.RTPport);
			LogMain("\nparse end:\nUsing IP: " + cfg.IP + " Media Path: " + cfg.MediaPath + " port: " + boost::to_string(cfg.port)+ " RTPPort: " + boost::to_string(cfg.RTPport));
		}

		//Инициализация адреса и порта сервера		
		boost::asio::io_service io_service;
		const udp::endpoint ep(boost::asio::ip::address::from_string(cfg.IP),cfg.port);
		const udp::endpoint SIPep(boost::asio::ip::address::from_string(cfg.IP), cfg.SIPport);
		///////////////////////////////////////////////////////////////////////////
		av_log_set_level(0);
		av_register_all();
		avcodec_register_all();
		avfilter_register_all();
		avformat_network_init();
		/************************************************************************
			Запуск экземляра MGCP-сервера	                                                                     
		************************************************************************/
		CMGCPServer s({ io_service, ep, SIPep, cfg.MediaPath });
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
		MessBox("Exception:3" + boost::to_string(e.what()));
		system("pause");
	}
	return 0;
}
