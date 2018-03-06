#include "stdafx.h"
#include "Structs.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
InitParams::InitParams(char* argv_)
{
	data.resize(MaxParamNames);
	data[RTPPort] = "20000";
	data[outerPort] = "2427";
	data[innerPort] = "2427";
	data[innerIP] = "127.0.0.1";
	data[MaxTimeAnn] = "90";
	data[MaxTimeCnf] = "1";
	data[MaxTimePrx] = "1";
	data[LogLevel] = "100";
	data[MediaPath] = "MediaFiles";
	GetPathExe(argv_);//Put path to exe in InitP
	ParseConfigFile();
}
//*///------------------------------------------------------------------------------------------
string InitParams::GetParams()
{
	string result = "";

	result += "\nNumber, from which RTP ports starts counting with step = 2: " + data[RTPPort];
	result += "\nOuter port: " + data[outerPort];
	result += "\nInner port: " + data[innerPort];
	result += "\nInner IP: " + data[innerIP];
	result += "\nMediaPath: " + data[PathEXE] + data[MediaPath];
	result += "\nOuter IP: " + data[outerIP];
	result += "\nMax inactive time for Announcements in mins: " + data[MaxTimeAnn];
	result += "\nMax inactive time for Conference in mins: " + data[MaxTimeCnf];
	result += "\nMax inactive time for Proxy calls in mins: " + data[MaxTimePrx];
	result += "\nLogLevel: " + data[LogLevel];
	result += "\nParsed path to application(home folder for MGCP server):\n" + data[PathEXE];
	return result+"\n";
}
//*///------------------------------------------------------------------------------------------
void InitParams::GetPathExe(char* argv_)
{
	using namespace boost::filesystem;
	path full_path(initial_path<path>());
	full_path = system_complete(path(argv_));
#ifdef WIN32
	data[PathEXE] = full_path.parent_path().string() + "\\";
#endif
#ifdef __linux__
	data[PathEXE] = full_path.parent_path().string() + "/";
#endif
}
//*///------------------------------------------------------------------------------------------
void InitParams::ParseConfigFile()
{
	ifstream file;
	file.open(data[PathEXE] + "MGCPInit.cfg");
	if (file.is_open())
	{
		string file_line;
		size_t found;
		vector<string> alphabet = { "RTPport=", "outerPort=", "innerPort=", "MediaPath=", "outerIP=", "innerIP=",
			"MaxTimeAnn=", "MaxTimeCnf=", "MaxTimePrx=", "LogLevel=" };
		while (getline(file, file_line) && (file_line.substr(0, 3) != "***"))
		{
			//cout << "\nDEBUG Structs::temp=" << temp;
			for (int i = 0; i < MaxParamNames; ++i)
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
	if (data[outerIP] == data[innerIP]) data[innerPort] = "2428";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Socket::Socket(string ip_, int port_, boost::asio::io_service& io_) :s(io_)
{
	using boost::asio::ip::udp;
	s.open(udp::v4());
	s.set_option(udp::socket::reuse_address(true));
	s.bind(udp::endpoint(boost::asio::ip::address::from_string(ip_), port_));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
OuterMes::~OuterMes()
{
	delete [] rawMes;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------