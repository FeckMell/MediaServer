#include "stdafx.h"
#include "Config.h"

map<string, string> CFG::mapData = {};

void CFG::Init(char* path_)
{
#ifdef WIN32
	mapData["Slash"] = "\\";
#else
	mapData["Slash"] = "/";
#endif
	mapData["Version"] = "3.0";

	GetPathEXE(string(path_));
	mapData["RoomSize"] = "15";
	mapData["RTPPort"] = "20000";
	mapData["MGCPPort"] = "2427";
	mapData["MaxTimeAnn"] = "90";
	mapData["MaxTimeCnf"] = "1";
	mapData["MaxTimePrx"] = "1";
	mapData["LogLevel"] = "info";
	mapData["MediaPath"] = "MediaFiles";
	mapData["SipPort"] = "5070";
	mapData["LogPath"] = Param("HomePath") + Param("Slash") + "MS_logs";

	mapData["MyFakeIP"] = "127.0.0.1";
	mapData["MyFakePort"] = "20000";
	mapData["OuterFakeIP"] = "1.1.1.1";
	mapData["OuterFakePort"] = "20000";

	ParseConfigFile();
	mapData["MediaPath"] = Param("HomePath") + Param("Slash") + Param("MediaPath");

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
	size_t fd_pos = path_.find_last_of(Param("Slash"));
	path_.resize(fd_pos);
	mapData["HomePath"] = path_;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void CFG::ParseConfigFile()
{
	ifstream file;
	file.open(Param("HomePath") + Param("Slash") + "ConfigMS.cfg");
	if (file.is_open())
	{
		string file_line;
		cmatch results;
		regex reg3(R"((\w+)=(.+))");

		while (getline(file, file_line) && (file_line.substr(0, 3) != "***"))
		{
			regex_match(file_line.c_str(), results, reg3);
			mapData[results.str(1)] = results.str(2);
		}
		if (Param("OuterIP") == "") mapData["Error"] = "IP for receiving messages not set in *.cfg file.\n";
		//else if (data["sipName"] == "") error = "sipName for receiving messages not set in *.cfg file.\n";
	}
	else { mapData["Error"] = "Could not open *.cfg file. Check its existance or name. Name Must \"be MGCPInit.cfg\".\n"; }
	if (Param("Error") != "")
	{
		cout << "\n" + Param("Error");
		system("pause");
		exit(-1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string CFG::PrintAll()
{
	//2TODO
	return "\n";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string CFG::Param(string name_)
{
	return mapData[name_];
}