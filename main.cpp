#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
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
		cout << "\nВЕРСИЯ 1.3.7 (15.06.2016 / 16:21)";
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
		///////////////////////////////////////////////////////////////////////////
		av_log_set_level(0);
		av_register_all();
		avcodec_register_all();
		avfilter_register_all();
		avformat_network_init();
		/************************************************************************
			Запуск экземляра MGCP-сервера	                                                                     
		************************************************************************/
		CMGCPServer s({ io_service, ep, cfg.MediaPath });
		LogMain("Запуск экземляра MGCP-сервера");
		cout << "\n-------------------------------------------------------------------------------\n\n";

		MusicPath = cfg.MediaPath;
		s.RunBuffer();
		while (true)
		{
			try
			{
				s.Run();
				//io_service.run();
			}
			catch (std::exception& e)
			{
				string es(e.what());
				if (es.find("receive_from") == std::string::npos) 
				{ throw e; }
			}
		}
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
