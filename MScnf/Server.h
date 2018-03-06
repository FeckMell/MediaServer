#pragma once
#include "../SharedSource/stdafx.h"
#include "../SharedSource/Structs.h"
#include "../SharedSource/InnerParser.h"
#include "Control.h"

extern SHP_STARTUP init_Params;

class InnerServer
{
public:
	InnerServer(int, string);
	void Run();
	void Receive(boost::system::error_code, size_t);
private:
	void ReplyError(SHP_IPL);

	SHP_Control iplManagement;
	REQUEST message = (REQUEST());
};