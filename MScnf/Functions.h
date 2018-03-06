#pragma once
#include "stdafx.h"
#include "Structs.h"
using namespace std;

extern SHP_IPar init_Params;
extern SHP_NETDATA net_Data;


//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string replace_in_str(string, string, string);
string get_substr(string, string, string);
string cut_substr(string, string, string);
string remove_from_str(string, string);
void init_ffmpeg();
void LogsInit();