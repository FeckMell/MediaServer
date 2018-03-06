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

Logger* CLogger = new Logger();
void Runner()
{
	CLogger->Run();
}
int main(int argc, char* argv[])
{
	try
	{
		timeBeginPeriod(1);
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		DateStr = GetDate();
		setlocale(LC_ALL, "Russian");
		cout << "\n���� ������ 1.4.2 (14.07.2016 / 12:17)";
		GetPathExe(argv[0]);
		CLogger->Create();
		boost::thread my_thread(&Runner);
		my_thread.detach();
		std::this_thread::sleep_for(std::chrono::microseconds(50));
		
		LogMain("Using path: " + PathEXE);
		/************************************************************************
			������� �������� ����������	                                                                     
		************************************************************************/
		string strConfigFile(PathEXE + "mgcpserver.cfg");
		Config cfg;
		cfg = ParseConfig(strConfigFile, cfg);
		RTPport = cfg.RTPport;
		if (cfg.error == -1){ cout << "\nNO MEDIA " << cfg.error; system("pause"); return 0; }
		if (cfg.error == 0)
		{
			cout << "\nparse end:\nUsing IP: " << cfg.IP << "\nMedia Path: " << cfg.MediaPath << "\nMGCPport: " << cfg.MGCPport << "\nSIPport: " << cfg.SIPport << "\nRTPPort: " << cfg.RTPport;
			LogMain("\nparse end:\nUsing IP: " + cfg.IP + "\nMedia Path: " + cfg.MediaPath + "\nMGCPport: " + boost::to_string(cfg.MGCPport) + "\nSIPport: " + boost::to_string(cfg.SIPport) + "\nRTPPort: " + boost::to_string(cfg.RTPport));
		}
		//������������� ������ � ����� �������		
		boost::asio::io_service io_service;
		const udp::endpoint MGCPep(boost::asio::ip::address::from_string(cfg.IP),cfg.MGCPport);
		const udp::endpoint SIPep(boost::asio::ip::address::from_string(cfg.IP), cfg.SIPport);
		///////////////////////////////////////////////////////////////////////////
		av_log_set_level(0);
		av_register_all();
		avcodec_register_all();
		avfilter_register_all();
		avformat_network_init();
		/************************************************************************
			������ ��������� MGCP-�������	                                                                     
		************************************************************************/
		/*CMGCPServer s({ io_service, MGCPep,SIPep, cfg.MediaPath });
		LogMain("������ ��������� MGCP-�������");
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
		}*/
		CMGCPServer s({ io_service, MGCPep, SIPep, cfg.MediaPath });
		LogMain("������ ��������� MGCP-�������");
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
