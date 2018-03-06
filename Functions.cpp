#include "stdafx.h"
#include "Functions.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
std::string GetDate()
{
	boost::gregorian::date Today = boost::gregorian::day_clock::local_day();
	return boost::to_string(Today.year()) + "-" + boost::to_string(Today.month().as_number()) + "-" + boost::to_string(Today.day().as_number());
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
string GetTime()
{
	//using namespace boost::posix_time;
	boost::posix_time::ptime t = boost::posix_time::second_clock::local_time();
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
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
				parsed.MediaPath = PathEXE + temp.substr(found + 6, temp.back());
				while (parsed.MediaPath.find("/") != std::string::npos)
				{
					parsed.MediaPath.replace(parsed.MediaPath.find("/"), 1, "\\");
				}
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
	CLogger.AddToLog(0, "\n"+a);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline void MessBoxHelper(string text)
{
	//MessageBox(NULL, L"Лалалал", L"Error", MB_OK);
	//out << "\nthread MessBox " << std::this_thread::get_id();
	MessageBoxA(NULL, text.c_str(), "Error", MB_OK);
}
void MessBox(string mess)
{
	boost::thread my_thread(&MessBoxHelper, mess);
	my_thread.detach();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
string MakeRemoteIP(string SDP)
{
	std::size_t found = SDP.find("c=IN IP4");
	if (found != std::string::npos)
	{
		std::string temp = SDP.substr(found + 9, SDP.find("\n", found + 1) - found - 9);
		found = temp.find("\n");
		if (found != std::string::npos)
		{
			temp = temp.substr(0, found);
		}

		string result = "";
		auto fd = temp.find(".");
		while (fd != std::string::npos)
		{
			string temp2 = temp.substr(0, fd);
			temp.erase(temp.begin(), temp.begin() + fd + 1);
			result += std::to_string(stoi(temp2)) + ".";
			fd = temp.find(".");
		}
		result += std::to_string(stoi(temp));
		temp = result;
		return temp;
	}
		//return SDP.substr(found + 9, SDP.find("\n", found + 1) - found - 9);
	return "";
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
string MakeRemotePort(string SDP)
{
	std::size_t found = SDP.find("m=audio");
	if (found != std::string::npos)
		return SDP.substr(found + 8, SDP.find(" ", found + 10) - found - 8);
	return "";
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void GetPathExe(char* argv)
{
	boost::filesystem::path full_path(boost::filesystem::initial_path<boost::filesystem::path>());
	full_path = boost::filesystem::system_complete(boost::filesystem::path(argv));
	PathEXE = full_path.parent_path().string() + "\\";
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int sdp_read(void *opaque, uint8_t *buf, int size) /*noexcept*/
{
	assert(opaque);
	assert(buf);
	auto octx = static_cast<SdpOpaque*>(opaque);

	if (octx->pos == octx->data.end())
		return 0;

	auto dist = static_cast<int>(std::distance(octx->pos, octx->data.end()));
	auto count = std::min(size, dist);
	std::copy(octx->pos, octx->pos + count, buf);
	octx->pos += count;

	return count;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------






































/*void logit(string a, int thread, string classname)
{
GetDate();
FILE* file;
string time = GetTime();
string filepath = PathEXE + "\\" + DateStr + "_" + classname + ".txt";
//out << "\nfilepath=" << filepath;
fopen_s(&file, filepath.c_str(), "a");
fprintf(file, (time + " thread=" + boost::to_string(thread) + "       " + a + "\n\n").c_str());
fflush(file);
fclose(file);
//out << "\nLOGIT DONE";
}*/