#include "stdafx.h"
#include "Functions.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void GetDate()
{
	boost::gregorian::date Today = boost::gregorian::day_clock::local_day();
	if (Today != Date)
	{
		Date = Today;
		DateStr = boost::to_string(Today.day().as_number()) + "-" + boost::to_string(Today.month().as_number()) + "-" + boost::to_string(Today.year());
		OpenLogFiles();
	}
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
string GetTime()
{
	//using namespace boost::posix_time;
	boost::posix_time::ptime t = boost::posix_time::second_clock::local_time();
	boost::chrono::steady_clock::time_point t1 = boost::chrono::steady_clock::now();
	return DateStr + "/" + boost::to_string(t.time_of_day()) + "/" + boost::to_string(t1.time_since_epoch().count() % 1000);
	
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
char *const get_error_text(const int error)
{
	static char error_buffer[255];
	av_strerror(error, error_buffer, sizeof(error_buffer));
	return error_buffer;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
string GetSelfIP()
{
	asio::io_service io_service;
	const auto strHostName = asio::ip::host_name();
		boost::system::error_code ec;
	udp::resolver resolver(io_service);
	udp::resolver::query query(udp::v4(), strHostName, "0");
	udp::resolver::iterator const end, itr = resolver.resolve(query, ec);
	if (ec)
		cout << "Error resolving myself: " << ec.message() << '\n';
	assert(itr != end);
	return itr->endpoint().address().to_string();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
Config ParseConfig(string path, Config parsed)
{
	int persistance = 0;
	std::ifstream file;
	string temp;
	std::size_t found;
	file.open(path);
	if (file.is_open()) {
		while (std::getline(file, temp))
		{
			found = temp.find("port=");
			if (found != std::string::npos)
			{
				parsed.port = stoi(temp.substr(found + 5, temp.back()));
				persistance = 1;
				break;
			}
		}
		if (persistance == 0) { parsed.port = 2427; }
		persistance = 0;
		file.close();
		file.open(path);
		while (std::getline(file, temp))
		{
			found = temp.find("mpath=");
			if (found != std::string::npos)
			{
				parsed.MediaPath = temp.substr(found + 6, temp.back());
				persistance = 1;
				break;
			}
		}
		if (persistance == 0) { parsed.error = -1; }
		persistance = 0;
		file.close();
		file.open(path);
		while (std::getline(file, temp))
		{
			found = temp.find("IP=");
			if (found != std::string::npos)
			{
				parsed.IP = temp.substr(found + 3, temp.back());
				persistance = 1;
				break;
			}
		}
		if (persistance == 0) { parsed.IP = GetSelfIP(); }
		persistance = 0;
		file.close();
	}
	else
	{
		parsed.error = -2;
		cout << "\n!!! config file not found !!!";
	}
	return parsed;


}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void LogMain(string a)
{

	fprintf(FileLog, (a + "\n").c_str());
	fflush(FileLog);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void OpenLogFiles()
{
	using namespace boost;
	gregorian::date TODAY = gregorian::day_clock::local_day();
	string date = to_string(TODAY.day().as_number()) + "-" + to_string(TODAY.month().as_number()) + "-" + to_string(TODAY.year()) + "_";
	string tempPath = PathEXE + date;
	fopen_s( &FileLog,           (tempPath + "LOGS.txt"          ).c_str(),"w");
	fopen_s( &FileLogConfPoint,  (tempPath + "LOGS_ConfPoint.txt").c_str(),"w");
	fopen_s( &FileLogConfRoom,   (tempPath + "LOGS_ConfRoom.txt" ).c_str(),"w");
	fopen_s( &FileLogMixer,      (tempPath + "LOGS_Mixer.txt"    ).c_str(),"w");
	fopen_s( &FileLogServer,     (tempPath + "LOGS_Server.txt"   ).c_str(),"w");
	fopen_s( &FileLogMixerInit,  (tempPath + "LOGS_MixerInit.txt").c_str(),"w");
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline void MessBoxHelper(string text)
{
	//MessageBox(NULL, L"Лалалал", L"Error", MB_OK);
	MessageBoxA(NULL, text.c_str(), "Error", MB_OK);
}
void MessBox(string mess)
{
	boost::thread my_thread(&MessBoxHelper, mess);
	my_thread.detach();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------







































/*void logit(string a, int thread, string classname)
{
GetDate();
FILE* file;
string time = GetTime();
string filepath = PathEXE + "\\" + DateStr + "_" + classname + ".txt";
//cout << "\nfilepath=" << filepath;
fopen_s(&file, filepath.c_str(), "a");
fprintf(file, (time + " thread=" + boost::to_string(thread) + "       " + a + "\n\n").c_str());
fflush(file);
fclose(file);
//cout << "\nLOGIT DONE";
}*/