#include "stdafx.h"
#include "SL_Logs.h"

vector<string> LOG::logNames = { "MAIN", "MGCP", "SIP", "ANN", "CNF", "PRX", "DTMF", "SQL" };
//vector<BOOSTLOGGER> LOG::vecLogs = {};
BOOSTLOGGER LOG::vecLogs;

void LOG::Init()
{
	//vecLogs.resize(maxL);
	//vecLogs.resize(1);

	//for (int i = 0; i < maxL; ++i)
	//{
		string log_path = CFG::data[CFG::homePath] + "\\logs\\%Y-%m-%d_" + logNames[0] + ".log";
	//string log_path ="\\logs\\%Y-%m-%d_.log";
		boost::log::add_file_log
			(
			boost::log::keywords::auto_flush = true,
			boost::log::keywords::file_name = log_path,                                        /*< file name pattern >*/
			boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 1), /*< ...or at midnight >*/
			boost::log::keywords::format = "[%TimeStamp%]:[%ThreadID%] %Message%",                                 /*< log record format >*/
			boost::log::keywords::open_mode = std::ios_base::app
			);
		boost::log::core::get()->set_filter(boost::log::trivial::severity >= stoi(CFG::data[CFG::logLevel]));
		boost::log::add_common_attributes();
	//}
}
BOOSTLOGGER LOG::GL(/*L*/int l_)
{
	//try{ return vecLogs[l_]; }
	try{ return vecLogs; }
	catch (exception& e)
	{
		cout << "\n Exception:" << e.what();
		system("pause");
		exit(-1);
	}
}