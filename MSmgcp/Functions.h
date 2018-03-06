#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "MGCPparser.h"

using namespace std;

extern SHP_IPar init_Params;
extern SHP_NETDATA net_Data;
extern src::severity_logger< logging::trivial::severity_level > lg;

class MGCP;
typedef shared_ptr<MGCP> SHP_MGCP;

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string replace_in_str(string, string, string);
string get_substr(string, string, string);
string cut_substr(string, string, string);
string remove_from_str(string, string);
void ReplyClient(SHP_MGCP, string);
void SendModul(int, string);
void LogsInit();
