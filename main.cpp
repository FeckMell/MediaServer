//#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <ostream>
#include <process.h>//////
#include <psapi.h>///////
//#include <vcl.h>//
#include "MGCPserver.h"

#include "iostream"
#include "windows.h"
#include "string.h"
#ifdef WIN32
//#include "vld.h" //visual leak detector
#endif // WIN32
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;
namespace opt = boost::program_options;
namespace cmdstyle = opt::command_line_style;
const auto cmdline_style = cmdstyle::unix_style | cmdstyle::allow_long_disguise;

static opt::options_description descConfig("Command line and config file options");
static opt::options_description descCmdLine("Command line options");
FILE *FileLog;
FILE *FileLogConfPoint;
FILE *FileLogConfRoom;
FILE *FileLogMixer;
FILE *FileLogMixerInit;
FILE *FileLogServer;

//extern FILE *FileLogConfPoint;

void LogMain(string a)
{
	fprintf(FileLog, a.c_str());
	fflush(FileLog);
}
int main(int argc, char* argv[])
{
	try
	{
		fs::path full_path(fs::initial_path<fs::path>());
		full_path = fs::system_complete(fs::path(argv[0]));
		string pathEXE = full_path.parent_path().string();
		string tempPath = pathEXE + "\\LOGS.txt";
		setlocale(LC_ALL, "Russian"/*"ru_RU.UTF-8"*/);
		fopen_s(&FileLog, tempPath.c_str(), "w");
		tempPath = pathEXE + "\\LOGS_ConfPoint.txt";
		fopen_s(&FileLogConfPoint, tempPath.c_str(), "w");
		tempPath = pathEXE + "\\LOGS_ConfRoom.txt";
		fopen_s(&FileLogConfRoom, tempPath.c_str(), "w");
		tempPath = pathEXE + "\\LOGS_Mixer.txt";
		fopen_s(&FileLogMixer, tempPath.c_str(), "w");
		tempPath = pathEXE + "\\LOGS_Server.txt";
		fopen_s(&FileLogServer, tempPath.c_str(), "w");
		tempPath = pathEXE + "\\LOGS_MixerInit.txt";
		fopen_s(&FileLogMixerInit, tempPath.c_str(), "w");
		LogMain("\nmain.cpp started");
		
		
		LogMain(full_path.string());
		/************************************************************************
			Парсинг входящих параметров	                                                                     
		************************************************************************/
		unsigned short port = 2427;
		string strMediaPath;
		//string strConfigFile("mgcpserver.cfg");
		string strConfigFile(pathEXE + "mgcpserver.cfg");

		//Параметры командной строки
		descCmdLine.add_options()
			("help,?", "produce help message")
			//("?", "produce help message")
			("cfg", opt::value<string>(&strConfigFile), "config file")
			;

		//Параметры в файле конфигурации
		descConfig.add_options()
			("port,p", opt::value<unsigned short>(&port)->default_value(port), "set MGCPserver port")
			//("mpath,m", opt::value<string>(&strMediaPath)->default_value("./MediaFiles"), "path to ann mediafiles")
			("mpath,m", opt::value<string>(&strMediaPath)->default_value(pathEXE + "\\MediaFiles"), "path to ann mediafiles")
			;

		//Объединение параметров для командной строки
		descCmdLine.add(descConfig);

		opt::variables_map vm; //Storage распарсенных опций
		string str(argv[0]);
		cout << "\n--------\n" << argv[0] << "\n-------\n";
		cout << "\n--------\n" << str << "\n-------\n";
		cout << "\n--------\n" << pathEXE << "\n-------\n";
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
			cout << boost::format("Merge options from config file %1%\n") % strConfigFile;
			store(parse_config_file(ifs, descConfig), vm);
			notify(vm);
		}
		
		//Инициализация адреса и порта сервера
		const auto strHostName = asio::ip::host_name();

		asio::io_service io_service;
		asio::io_service io_service1;
		udp::resolver resolver(io_service);
		udp::resolver::query query(udp::v4(), strHostName, boost::lexical_cast<string>(port));
		udp::resolver::iterator const end, itr = resolver.resolve(query);
		LogMain("has been resolved as V4 ip's:\n");
		cout << strHostName << " has been resolved as V4 ip's:\n";
		if (itr == end)
		{
			LogMain("Can't resolve myself\n");
			cerr << boost::format("Can't resolve myself <%1%>\n") % strHostName;
			return 1;
		}
		std::for_each(itr, end, 
			[](const udp::endpoint& ep){cout << '\t' << ep << std::endl; });

		const udp::endpoint& endpServer = *itr;
		LogMain("Using:Server endpnt and Media path");
		cout << boost::format("Using:\n\tServer endpnt\t%1%\n\tMedia path\t%2%\n\n") 
			% endpServer % strMediaPath;

		/************************************************************************
			Запуск экземляра MGCP-сервера	                                                                     
		************************************************************************/
		CMGCPServer s({ io_service, endpServer, strMediaPath, io_service1 });
		LogMain("Запуск экземляра MGCP-сервера	\n");
		s.Run();
		//
		io_service.run();
	}
	catch (std::exception& e)
	{
		LogMain("Exception:");
		cerr << "Exception: " << e.what() << "\n";
	}
	fclose(FileLog);
	system("pause");
	return 0;
}
