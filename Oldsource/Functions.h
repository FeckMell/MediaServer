#pragma once
#ifdef WIN32
#include "stdafx.h"
#include <boost/type_index.hpp>
#endif
#ifdef linux
#include "stdinclude.h"
#endif
#include "Structs.h"
#include "Logger.h"
#include "boost/date_time/posix_time/posix_time.hpp"
class Logger;
extern boost::gregorian::date Date;
extern string DateStr;
extern string PathEXE;
extern Logger* CLogger;


//----------------------------------------------------------------------------
std::string GetDate();
//----------------------------------------------------------------------------
string GetTime();
//----------------------------------------------------------------------------
char *const get_error_text(const int error);
//----------------------------------------------------------------------------
string GetSelfIP();
//----------------------------------------------------------------------------
Config ParseConfig(string path, Config parsed);
//----------------------------------------------------------------------------
void LogMain(string a);
//----------------------------------------------------------------------------
void MessBox(string mess);
//----------------------------------------------------------------------------
string MakeRemotePort(string SDP);
//----------------------------------------------------------------------------
string MakeRemoteIP(string SDP);
//----------------------------------------------------------------------------
void GetPathExe(char* argv);
//----------------------------------------------------------------------------
int sdp_read(void *opaque, uint8_t *buf, int size);
//----------------------------------------------------------------------------
std::string get_substr(std::string target, std::string aim, std::string fin);
//----------------------------------------------------------------------------
std::string cut_substr(std::string target, std::string aim, std::string fin);
//*///------------------------------------------------------------------------------------------
std::string remove_from_str(std::string target, std::string aim);
//*///------------------------------------------------------------------------------------------
std::string replace_in_str(std::string target, std::string what, std::string to_what);
















/*template <class T>
string GetClass(T t)
{
std::string name = boost::typeindex::type_id_with_cvr<decltype(t)>().pretty_name();
return name.substr(name.find_last_of(" ") + 1, name.back());
}*/
/*void logit(string a, int thread, string classname);*/





