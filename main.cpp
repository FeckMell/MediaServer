#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
//#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "MGCPserver.h"



#ifdef WIN32
//#include "vld.h" //visual leak detector
#endif // WIN32

namespace fs = boost::filesystem;
namespace opt = boost::program_options;
namespace cmdstyle = opt::command_line_style;
const auto cmdline_style = cmdstyle::unix_style | cmdstyle::allow_long_disguise;

//static opt::options_description descConfig("Command line and config file options");
static opt::options_description descCmdLine("Command line options");
FILE *FileLog;
FILE *FileLogConfPoint;
FILE *FileLogConfRoom;
FILE *FileLogMixer;
FILE *FileLogMixerInit;
FILE *FileLogServer;
boost::gregorian::date Date;
string DateStr;
string PathEXE0;

//extern FILE *FileLogConfPoint;
void LogMain(string a)
{

	fprintf(FileLog, (a+"\n").c_str());
	fflush(FileLog);
}
void OpenLogFiles(string path)
{
	boost::gregorian::date TODAY = boost::gregorian::day_clock::local_day();
	string date = to_string(TODAY.day().as_number()) + "-" + to_string(TODAY.month().as_number()) + "-" + to_string(TODAY.year()) + "_";
	string tempPath;
	PathEXE0 = path;
	tempPath = path/* + "\\"*/ + date + "LOGS1.txt";
	fopen_s(&FileLog, tempPath.c_str(), "w");
	tempPath = path/* + "\\"*/ + date + "LOGS_ConfPoint1.txt";
	fopen_s(&FileLogConfPoint, tempPath.c_str(), "w");
	tempPath = path/* + "\\"*/ + date + "LOGS_ConfRoom1.txt";
	fopen_s(&FileLogConfRoom, tempPath.c_str(), "w");
	tempPath = path/* + "\\"*/ + date + "LOGS_Mixer1.txt";
	fopen_s(&FileLogMixer, tempPath.c_str(), "w");
	tempPath = path/* + "\\"*/ + date + "LOGS_Server1.txt";
	fopen_s(&FileLogServer, tempPath.c_str(), "w");
	tempPath = path/* + "\\"*/ + date + "LOGS_MixerInit1.txt";
	fopen_s(&FileLogMixerInit, tempPath.c_str(), "w");
}

int main(int argc, char* argv[])
{
	try
	{
		/*get filepath*/
		fs::path full_path(fs::initial_path<fs::path>());
		full_path = fs::system_complete(fs::path(argv[0]));
		PathEXE0 = full_path.parent_path().string()+"\\";

		setlocale(LC_ALL, "Russian"/*"ru_RU.UTF-8"*/);
		OpenLogFiles(PathEXE0);

		LogMain(full_path.string());
		/************************************************************************
			Парсинг входящих параметров	                                                                     
		************************************************************************/
		unsigned short port = 2427;
		string strMediaPath;
		string IP;
		//string strConfigFile("mgcpserver.cfg");
		string strConfigFile(PathEXE0 + "mgcpserver.cfg");
		Config cfg;
		cfg = ParseConfig(strConfigFile, cfg);
		cout << "\nparse end:\n" << cfg.IP <<" " << cfg.MediaPath<< " " << cfg.port;
		if (cfg.error == -1){ cout << "\nNO MEDIA " << cfg.error; system("pause"); return 0; }



		//Параметры командной строки
		descCmdLine.add_options()
			("help,?", "produce help message")
			//("?", "produce help message")
			("cfg", opt::value<string>(&strConfigFile), "config file")
			;

		//Параметры в файле конфигурации
		/*descConfig.add_options()
			("port,p", opt::value<unsigned short>(&port)->default_value(port), "set MGCPserver port")
			//("mpath,m", opt::value<string>(&strMediaPath)->default_value("./MediaFiles"), "path to ann mediafiles")
			("mpath,m", opt::value<string>(&strMediaPath)->default_value(PathEXE0 + "MediaFiles"), "path to ann mediafiles")
			("IP,I", opt::value<string>(&IP)->default_value("10.77.7.19"), "set MGCPserver IP")
			;*/

		//Объединение параметров для командной строки
		//descCmdLine.add(descConfig);

		opt::variables_map vm; //Storage распарсенных опций
		string str(argv[0]);
		//Парсинг командной строки
		opt::store(opt::parse_command_line(argc, argv, descCmdLine, cmdline_style), vm);
		opt::notify(vm);
		if (vm.count("help") || vm.count("?"))
		{
			LogMain("========== Options available ==========\n return 1");

			cout << "========== Options available ==========\n" 
				<< descCmdLine << "\n"; //print usage			
			return 1;
		}
		
		//Парсинг файла конфигурации 
		//(если указан явно или существует с именем по умолчанию)
		std::ifstream ifs(strConfigFile.c_str());
		//std::ifstream ifs(PathConfig);
		if (!ifs)
		{
			LogMain("if (!ifs)==true");
			//Не открылся
			if (vm.count("cfg"))//Сообщение об ошибке, если имя было указано в опциях
			{
				LogMain("can not open config file: return 0");
				cout << "can not open config file: " << strConfigFile << "\n";
				return 0;
			}
		}
		else
		{
			LogMain("Merge options from config file \n");
			cout << boost::format("\nMerge options from config file %1%\n") % strConfigFile;
			//store(parse_config_file(ifs, descConfig), vm);
			notify(vm);
		}
		
		//Инициализация адреса и порта сервера		
		asio::io_service io_service;
		const udp::endpoint ep(boost::asio::ip::address::from_string(cfg.IP),cfg.port);
		LogMain("Using:Server endpnt // Media path:" + cfg.IP + "//" + cfg.MediaPath);
		/************************************************************************
			Запуск экземляра MGCP-сервера	                                                                     
		************************************************************************/
		CMGCPServer s({ io_service, /*endpServer*/ep, /*strMediaPath*/cfg.MediaPath });
		LogMain("Запуск экземляра MGCP-сервера	\n");
		s.Run();
		io_service.run();
	}
	catch (std::exception& e)
	{
		LogMain("Exception:");
		cerr << "Exception: " << e.what() << "\n";
		system("pause");
	}
	return 0;
}
