#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "InnerParser.h"

using namespace std;

extern SHP_IPar init_Params;
//extern boost::asio::io_service io_Server;
extern boost::asio::io_service io_Apps;
//extern SHP_Socket outer_Socket;
extern SHP_Socket inner_Socket;


//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string replace_in_str(string, string, string);
string get_substr(string, string, string);
string cut_substr(string, string, string);
string remove_from_str(string, string);
void init_ffmpeg();