#include "stdafx.h"
#include "Config.h"

vector<string> CFG::data = {};
string CFG::error = "";

void CFG::Init(char* path_)
{
	data.resize(maxParamNames);
	data[rtpPort] = "20000";
	data[mgcpPort] = "2427";
	data[innerPort] = "2427";
	data[innerIP] = "127.0.0.1";
	data[maxTimeAnn] = "90";
	data[maxTimeCnf] = "1";
	data[maxTimePrx] = "1";
	data[logLevel] = "3";
	data[mediaPath] = "MediaFiles";
	data[sipPort] = "5060";
	GetPathEXE(string(path_));//Put path to exe in InitP
	ParseConfigFile();
	data[mediaPath] = data[homePath] + "\\" + data[mediaPath];

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
	size_t fd_pos = path_.find_last_of("\\");
	path_.resize(fd_pos);
	data[homePath] = path_;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void CFG::ParseConfigFile()
{
	ifstream file;
	file.open(data[homePath] + "\\MGCPInit.cfg");
	if (file.is_open())
	{
		string file_line;
		size_t found;
		vector<string> alphabet = { "outerIP=", "innerIP=", "innerPort=", "logLevel=", "outerPort=", "rtpPort=",
			"maxTimeAnn=", "maxTimeCnf=", "maxTimePrx=", "mediaPath=", "portSIP=" };

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
string CFG::GetParams()
{
	string result = "";

	result += "\nNumber, from which RTP ports starts counting with step = 2: " + data[rtpPort];
	result += "\nOuter port: " + data[mgcpPort];
	result += "\nInner port: " + data[innerPort];
	result += "\nInner IP: " + data[innerIP];
	result += "\nMediaPath: " + data[mediaPath];
	result += "\nOuter IP: " + data[outerIP];
	result += "\nMax inactive time for Announcements in mins: " + data[maxTimeAnn];
	result += "\nMax inactive time for Conference in mins: " + data[maxTimeCnf];
	result += "\nMax inactive time for Proxy calls in mins: " + data[maxTimePrx];
	result += "\nLogLevel: " + data[logLevel];
	result += "\nportSIP: " + data[sipPort];
	result += "\nParsed path to application(home folder for MGCP server):\n" + data[homePath];
	return result + "\n";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------