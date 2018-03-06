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

namespace opt = boost::program_options;
namespace cmdstyle = opt::command_line_style;
const auto cmdline_style = cmdstyle::unix_style | cmdstyle::allow_long_disguise;

static opt::options_description descConfig("Command line and config file options");
static opt::options_description descCmdLine("Command line options");

FILE *FileLog;

int main(int argc, char* argv[])
{
	/*wchar_t szPath[MAX_PATH + 1];
	GetModuleFileNameW(0, szPath, sizeof(szPath) / sizeof(*szPath));

	Затем, для нормальной работы со строками используйте std::wstring, вы ж пишете на C++.

		std::wstring path(szPath);
	auto lastPos = path.find_last_of(L'\\');
	if (lastPos != std::wstring::npos)
		path = path.substr(lastPos + 1);*/
	try
	{
		setlocale(LC_ALL, "Russian"/*"ru_RU.UTF-8"*/);
		char szPath[200];
		GetModuleFileName(0, (LPWSTR)szPath, 200);
		printf("Filepath: %ws\n", szPath);
		char Path[200];
		int counter1 = 0;
		int counter2 = 0;
		for (int i = 0; i < 512; ++i)
		{
			if (szPath[i] == '\\')
			{
				counter1 = counter2;
			}
			if (szPath[i] != szPath[1])
			{
				Path[counter2] = szPath[i];
				++counter2;
			}
			if (szPath[i] == '.')
			{
				break;
			}
		}
		Path[counter1 + 1] = '\0';

		char PathLog[200];
		strcpy(PathLog, Path);

		cout << "\n";
		cout << strcat(PathLog, "LOGS.txt") << "\n";

		fopen_s(&FileLog, PathLog/*"LOGS.txt"*/, "w");
		/************************************************************************
			Парсинг входящих параметров			                                                                     
		************************************************************************/
		unsigned short port = 2427;
		string strMediaPath;
		string strConfigFile("mgcpserver.cfg");
		char PathConfig[200];
		strcpy(PathConfig, Path);
		cout << strcat(PathConfig, "mgcpserver.cfg") << "\n";
		char PathMedia[200];
		strcpy(PathMedia, Path);
		cout << strcat(PathMedia, "MediaFiles\\") << "\n";

		//Параметры командной строки
		descCmdLine.add_options()
			("help,?", "produce help message")
			//("?", "produce help message")
			("cfg", opt::value<char>(PathConfig/*&strConfigFile*/), "config file")
			;

		//Параметры в файле конфигурации
		descConfig.add_options()
			("port,p", opt::value<unsigned short>(&port)->default_value(port), "set MGCPserver port")
			("mpath,m", opt::value<string>(&strMediaPath)->default_value(PathMedia/*"./MediaFiles"*/), "path to ann mediafiles")
			;

		//Объединение параметров для командной строки
		descCmdLine.add(descConfig);

		opt::variables_map vm; //Storage распарсенных опций

		//Парсинг командной строки
		opt::store(opt::parse_command_line(argc, argv, descCmdLine, cmdline_style), vm);
		opt::notify(vm);
		if (vm.count("help") || vm.count("?"))
		{
			fprintf(FileLog, "========== Options available ==========\n return 1");
			cout << "========== Options available ==========\n" 
				<< descCmdLine << "\n"; //print usage			
			return 1;
		}
		
		//Парсинг файла конфигурации 
		//(если указан явно или существует с именем по умолчанию)
		//std::ifstream ifs(strConfigFile.c_str());
		std::ifstream ifs(PathConfig);
		if (!ifs)
		{
			fprintf(FileLog, "if (!ifs)==true");
			//Не открылся
			if (vm.count("cfg"))//Сообщение об ошибке, если имя было указано в опциях
			{
				fprintf(FileLog, "can not open config file: return 0");
				cout << "can not open config file: " << PathConfig/*strConfigFile*/ << "\n";
				return 0;
			}
		}
		else
		{
			fprintf(FileLog, "Merge options from config file \n");
			cout << boost::format("Merge options from config file %1%\n") % PathConfig/*strConfigFile*/;
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
		fprintf(FileLog, " has been resolved as V4 ip's:\n");
		cout << strHostName << " has been resolved as V4 ip's:\n";
		if (itr == end)
		{
			fprintf(FileLog, "Can't resolve myself\n");
			cerr << boost::format("Can't resolve myself <%1%>\n") % strHostName;
			return 1;
		}
		std::for_each(itr, end, 
			[](const udp::endpoint& ep){cout << '\t' << ep << std::endl; });

		const udp::endpoint& endpServer = *itr;
		fprintf(FileLog, "Using:\n\tServer endpnt\t\n\tMedia path\t\n\n");
		cout << boost::format("Using:\n\tServer endpnt\t%1%\n\tMedia path\t%2%\n\n") 
			% endpServer % strMediaPath;

		/************************************************************************
			Запуск экземляра MGCP-сервера	                                                                     
		************************************************************************/
		CMGCPServer s({ io_service, endpServer, strMediaPath, io_service1 }, FileLog);
		s.Run();
		//
		io_service.run();
	}
	catch (std::exception& e)
	{
		fprintf(FileLog, "Exception:");
		cerr << "Exception: " << e.what() << "\n";
	}
	fclose(FileLog);
	system("pause");
	return 0;
}
