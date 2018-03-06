#include "Structs.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
STARTUP::STARTUP(char* argv_)
{
	data.resize(maxParamNames);
	data[rtpPort] = "20000";
	data[outerPort] = "2427";
	data[innerPort] = "2427";
	data[innerIP] = "127.0.0.1";
	data[maxTimeAnn] = "90";
	data[maxTimeCnf] = "1";
	data[maxTimePrx] = "1";
	data[logLevel] = "3";
	data[mediaPath] = "MediaFiles";
	GetPathExe(argv_);//Put path to exe in InitP
	ParseConfigFile();
	data[mediaPath] = data[homePath] + "\\" + data[mediaPath];
}
//*///------------------------------------------------------------------------------------------
string STARTUP::GetParams()
{
	string result = "";

	result += "\nNumber, from which RTP ports starts counting with step = 2: " + data[rtpPort];
	result += "\nOuter port: " + data[outerPort];
	result += "\nInner port: " + data[innerPort];
	result += "\nInner IP: " + data[innerIP];
	result += "\nMediaPath: " + data[mediaPath];
	result += "\nOuter IP: " + data[outerIP];
	result += "\nMax inactive time for Announcements in mins: " + data[maxTimeAnn];
	result += "\nMax inactive time for Conference in mins: " + data[maxTimeCnf];
	result += "\nMax inactive time for Proxy calls in mins: " + data[maxTimePrx];
	result += "\nLogLevel: " + data[logLevel];
	result += "\nParsed path to application(home folder for MGCP server):\n" + data[homePath];
	return result + "\n";
}
//*///------------------------------------------------------------------------------------------
void STARTUP::GetPathExe(char* argv_)
{
	using namespace boost::filesystem;
	path full_path(initial_path<path>());
	full_path = system_complete(path(argv_));
#ifdef WIN32
	data[homePath] = full_path.parent_path().string();
#endif
#ifdef __linux__
	data[homePath] = full_path.parent_path().string();
#endif
}
//*///------------------------------------------------------------------------------------------
void STARTUP::ParseConfigFile()
{
	ifstream file;
	file.open(data[homePath] + "\\MGCPInit.cfg");
	if (file.is_open())
	{
		string file_line;
		size_t found;
		vector<string> alphabet = { "outerIP=", "innerIP=", "innerPort=", "logLevel=", "outerPort=", "rtpPort=",
			"maxTimeAnn=", "maxTimeCnf=", "maxTimePrx=", "mediaPath=" };

		while (getline(file, file_line) && (file_line.substr(0, 3) != "***"))
		{
			for (int i = 0; i < maxParamNames; ++i)
			{
				if ((found = file_line.find(alphabet[i])) != string::npos)
				{
					data[i] = file_line.substr(found + alphabet[i].length(), file_line.back());
					break;
				}
			}
		}
		if (data[outerIP] == ""){ error = "IP for receiving messages not set in *.cfg file.\n"; }
#ifdef __linux__
		data[PathEXE].pop_back();
		data[PathEXE].pop_back();
#endif
	}
	else { error = "Could not open *.cfg file. Check its existance or name. Name Must \"be MGCPInit.cfg\".\n"; }
	if (error != "")
	{
		cout << "\n" + error;
		system("pause");
		exit(-1);
	}
	if (data[outerIP] == data[innerIP]) data[innerPort] = "2327";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
/*void LogsInit(string my_modul_name_)
{
	string log_path = init_Params->data[STARTUP::homePath] + "\\logs\\%Y-%m-%d_" + my_modul_name_ + ".log";
	boost::log::add_file_log
		(
		boost::log::keywords::auto_flush = true,
		boost::log::keywords::file_name = log_path,                                        //< file name pattern >
		boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 1), //< ...or at midnight >
		boost::log::keywords::format = "[%TimeStamp%]:[%ThreadID%] %Message%",                                 //< log record format >
		boost::log::keywords::open_mode = std::ios_base::app
		);
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= stoi(init_Params->data[STARTUP::logLevel]));
	boost::log::add_common_attributes();
}*/