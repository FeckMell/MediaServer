#pragma once
#include "stdafx.h"
#include "Structs.h"

extern SHP_STARTUP init_Params;
extern SHP_NETDATA net_Data;
extern BOOSTLOGGER lg;

string replace_in_str(string, string, string);
string get_substr(string, string, string);
string cut_substr(string, string, string);
string remove_from_str(string, string);
string copy_single_n_line(string, int);
string remove_single_line(string, string);
string remove_single_line(string, int);
string replace_line(string, string, string);
int find_line(string, string);

void LogsInit(string);
void init_ffmpeg();
