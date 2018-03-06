#include "stdafx.h"
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>

#include "MGCPserver.h"
#ifdef WIN32
//#include "vld.h" //visual leak detector
#endif // WIN32

namespace opt = boost::program_options;
namespace cmdstyle = opt::command_line_style;
const auto cmdline_style = cmdstyle::unix_style | cmdstyle::allow_long_disguise;

static opt::options_description descConfig("Command line and config file options");
static opt::options_description descCmdLine("Command line options");



int main(int argc, char* argv[])
{
	try
	{

		setlocale(LC_ALL, "Russian"/*"ru_RU.UTF-8"*/);

		/************************************************************************
			Парсинг входящих параметров			                                                                     
		************************************************************************/
		unsigned short port = 2427;
		string strMediaPath;
		string strConfigFile("mgcpserver.cfg");

		//Параметры командной строки
		descCmdLine.add_options()
			("help,?", "produce help message")
			//("?", "produce help message")
			("cfg", opt::value<string>(&strConfigFile), "config file")
			;

		//Параметры в файле конфигурации
		descConfig.add_options()
			("port,p", opt::value<unsigned short>(&port)->default_value(port), "set MGCPserver port")
			("mpath,m", opt::value<string>(&strMediaPath)->default_value("./MediaFiles"), "path to ann mediafiles")
			;

		//Объединение параметров для командной строки
		descCmdLine.add(descConfig);

		opt::variables_map vm; //Storage распарсенных опций

		//Парсинг командной строки
		opt::store(opt::parse_command_line(argc, argv, descCmdLine, cmdline_style), vm);
		opt::notify(vm);
		if (vm.count("help") || vm.count("?"))
		{
			cout << "========== Options available ==========\n" 
				<< descCmdLine << "\n"; //print usage			
			return 1;
		}
		
		//Парсинг файла конфигурации 
		//(если указан явно или существует с именем по умолчанию)
		std::ifstream ifs(strConfigFile.c_str());
		if (!ifs)
		{
			//Не открылся
			if (vm.count("cfg"))//Сообщение об ошибке, если имя было указано в опциях
			{
				cout << "can not open config file: " << strConfigFile << "\n";
				return 0;
			}
		}
		else
		{
			cout << boost::format("Merge options from config file %1%\n") % strConfigFile;
			store(parse_config_file(ifs, descConfig), vm);
			notify(vm);
		}
		
		//Инициализация адреса и порта сервера
		const auto strHostName = asio::ip::host_name();

		asio::io_service io_service;
		udp::resolver resolver(io_service);
		udp::resolver::query query(udp::v4(), strHostName, boost::lexical_cast<string>(port));
		udp::resolver::iterator const end, itr = resolver.resolve(query);

		cout << strHostName << " has been resolved as V4 ip's:\n";
		if (itr == end)
		{
			cerr << boost::format("Can't resolve myself <%1%>\n") % strHostName;
			return 1;
		}
		std::for_each(itr, end, 
			[](const udp::endpoint& ep){cout << '\t' << ep << std::endl; });

		const udp::endpoint& endpServer = *itr;
		cout << boost::format("Using:\n\tServer endpnt\t%1%\n\tMedia path\t%2%\n\n") 
			% endpServer % strMediaPath;

		/************************************************************************
			Запуск экземляра MGCP-сервера	                                                                     
		************************************************************************/
		CMGCPServer s({ io_service, endpServer, strMediaPath });
		s.Run();

		//io_service.run();
	}
	catch (std::exception& e)
	{
		cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}
