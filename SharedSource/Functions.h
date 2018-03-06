#pragma once
#include "stdafx.h"
#include "Structs.h"

extern SHP_IPar init_Params;
extern SHP_NETDATA net_Data;
extern src::severity_logger<severity_level> lg;

string replace_in_str(string, string, string);
string get_substr(string, string, string);
string cut_substr(string, string, string);
string remove_from_str(string, string);

void LogsInit(string);
void init_ffmpeg();
