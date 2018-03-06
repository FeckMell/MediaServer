#pragma once
#include "stdafx.h"
#include "Structs.h"
#include <boost/type_index.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

extern boost::gregorian::date Date;
extern string DateStr;
extern string PathEXE;
extern FILE *FileLog;
extern FILE *FileLogConfPoint;
extern FILE *FileLogConfRoom;
extern FILE *FileLogMixer;
extern FILE *FileLogMixerInit;
extern FILE *FileLogServer;
/*FILE *FileLog2;
FILE *FileLogConfPoint2;
FILE *FileLogConfRoom2;
FILE *FileLogMixer2;
FILE *FileLogMixerInit2;
FILE *FileLogServer2;*/

//----------------------------------------------------------------------------
void GetDate();
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
void OpenLogFiles();
//----------------------------------------------------------------------------
void MessBox(string mess);
//----------------------------------------------------------------------------















/*template <class T>
string GetClass(T t)
{
std::string name = boost::typeindex::type_id_with_cvr<decltype(t)>().pretty_name();
return name.substr(name.find_last_of(" ") + 1, name.back());
}*/
/*void logit(string a, int thread, string classname);*/





