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
	std::ifstream file;
	string temp;
	std::size_t found;
	file.open(path);
	if (file.is_open())
	{
	    while (std::getline(file, temp))
        {
            if((found = temp.find("RTPport="))!=std::string::npos)
            {
                parsed.RTPport = stoi(temp.substr(found + 8, temp.back()));
                //parsed.RTPport = remove_from_str(parsed.RTPport,"\r");
                continue;
            }
            else if((found = temp.find("MGCPport="))!=std::string::npos)
            {
                parsed.MGCPport = stoi(temp.substr(found + 9, temp.back()));
                //parsed.MGCPport = remove_from_str(parsed.MGCPport,"\r");
                continue;
            }
            else if((found = temp.find("mpath="))!=std::string::npos)
            {
#ifdef WIN32
				parsed.MediaPath = PathEXE + temp.substr(found + 6, temp.back());
#endif
#ifdef __linux__
                parsed.MediaPath = /*PathEXE +*/ temp.substr(found + 6, temp.back());
#endif

                parsed.MediaPath = remove_from_str(parsed.MediaPath,"\r");
                continue;
            }
            else if((found = temp.find("IP="))!=std::string::npos)
            {
                parsed.IP = temp.substr(found + 3, temp.back());
                parsed.IP = remove_from_str(parsed.IP,"\r");
                continue;
            }
        }
        if(parsed.IP == "") { parsed.IP = GetSelfIP(); }
        if(parsed.MediaPath == "") { parsed.error = -1; }
	}
	else
	{
		parsed.error = -2;
		cout << "\n!!! config file not found !!!";
	}
	file.close();
	return parsed;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void LogMain(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	string result = DateStr + "/" + boost::to_string(t->tm_hour) + ":" + boost::to_string(t->tm_min) + ":" + boost::to_string(t->tm_sec) + "/" + boost::to_string(t1.time_since_epoch().count() % 1000);
	result += " thread=" + boost::to_string(std::this_thread::get_id()) + "      ";
	CLogger->AddToLog(0, result + a);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#ifdef WIN32
inline void MessBoxHelper(string text)
{
	MessageBoxA(NULL, text.c_str(), "Error", MB_OK);
}
void MessBox(string mess)
{
	boost::thread my_thread(&MessBoxHelper, mess);
	my_thread.detach();
}
#endif
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
	return get_substr(SDP, "m=audio ", " ");
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void GetPathExe(char* argv)
{
	boost::filesystem::path full_path(boost::filesystem::initial_path<boost::filesystem::path>());
	full_path = boost::filesystem::system_complete(boost::filesystem::path(argv));
#ifdef WIN32
	PathEXE = full_path.parent_path().string() + "\\";
#endif
#ifdef __linux__
	PathEXE = full_path.parent_path().string() + "/";
#endif

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
//вырезает после начального слова
std::string get_substr(std::string target, std::string aim, std::string fin)//target - откуда вырезаем, aim - начало, fin - конец
{
	auto fd = target.find(aim);
	std::string result = "";
	if (fd != std::string::npos)
		result = target.substr(fd + aim.size(), target.find(fin, fd + aim.size() - 1) - (fd + aim.size()));
	return result;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//вырезает полностью строку
std::string cut_substr(std::string target, std::string aim, std::string fin)
{
	auto fd = target.find(aim);
	std::string result = "";
	if (fd != std::string::npos)
		result = target.substr(fd, target.find(fin, fd + 1) - (fd));
	return result;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
std::string remove_from_str(std::string target, std::string aim)
{
    std::size_t fd;
    while ((fd=target.find(aim))!=std::string::npos)
    {
        target=target.erase(fd,fd+aim.size()-1);
    }
    return target;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
std::string replace_in_str(std::string target, std::string what, std::string to_what)
{
    std::size_t fd;
    std::vector<std::size_t> vec_pos;
    vec_pos.push_back(-1);
    while ((fd=target.find(what,fd+1))!=std::string::npos)
    {
        vec_pos.push_back(fd);
    }
    for(int i = vec_pos.size()-1; i > 0; --i)
    {
        target=target.replace(vec_pos[i],what.size()+1,to_what);
    }
    return target;
}




































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
