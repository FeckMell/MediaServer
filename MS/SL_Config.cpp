#include "stdafx.h"
#include "SL_Config.h"

map<string, string> CFG::data = {};
string CFG::error = "";
string CFG::version = "2.1.1 23.01.2017";
#ifdef WIN32
string CFG::slash = "\\";
#else
string CFG::slash = "/";
#endif

void CFG::Init(char* path_)
{
	GetPathEXE(string(path_));
	data["rtpPort"] = "20000";
	data["mgcpPort"] = "2427";
	data["maxTimeAnn"] = "90";
	data["maxTimeCnf"] = "1";
	data["maxTimePrx"] = "1";
	data["logLevel"] = "3";
	data["mediaPath"] = "MediaFiles";
	data["sipPort"] = "5070";
	data["logPath"] = data["homePath"] + slash + "MS_logs";
	data["logPort"] = "62975";

	ParseConfigFile();
	data["mediaPath"] = data["homePath"] + slash + data["mediaPath"];

	/* Init FFMPEG */
	av_log_set_level(0);
	av_register_all();
	avcodec_register_all();
	avfilter_register_all();
	avformat_network_init();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void CFG::GetPathEXE(string path_)
{
	size_t fd_pos = path_.find_last_of(slash);
	path_.resize(fd_pos);
	data["homePath"] = path_;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void CFG::ParseConfigFile()
{
	ifstream file;
	file.open(data["homePath"] + slash + "MGCPInit.cfg");
	if (file.is_open())
	{
		string file_line;
		cmatch results;
		regex reg3(R"((\w+)=(.+))");

		while (getline(file, file_line) && (file_line.substr(0, 3) != "***"))
		{
			regex_match(file_line.c_str(), results, reg3);
			data[results.str(1)] = results.str(2);
		}
		if (data["outerIP"] == "") error = "IP for receiving messages not set in *.cfg file.\n";
		//else if (data["sipName"] == "") error = "sipName for receiving messages not set in *.cfg file.\n";
	}
	else { error = "Could not open *.cfg file. Check its existance or name. Name Must \"be MGCPInit.cfg\".\n"; }
	if (error != "")
	{
		cout << "\n" + error;
		system("pause");
		exit(-1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string CFG::GetParams()
{
	string result = "VERSION=" + version;
	result += "\nParsed path to application(home folder for MGCP server):\n" + data["homePath"];
	result += "\nMediaPath: " + data["mediaPath"];
	result += "\nLogPath: " + data["logPath"];
	result += "\nOuter IP: " + data["outerIP"];
	result += "\nMGCP port: " + data["mgcpPort"];
	result += "\nSIP port: " + data["sipPort"];
	result += "\nNumber, from which RTP ports starts counting with step = 2: " + data["rtpPort"];

	result += "\n\nMax inactive time for Announcements in mins: " + data["maxTimeAnn"];
	result += "\nMax inactive time for Conference in mins: " + data["maxTimeCnf"];
	result += "\nMax inactive time for Proxy calls in mins: " + data["maxTimePrx"];
	result += "\nLogLevel: " + data["logLevel"];

	return result + "\n";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------