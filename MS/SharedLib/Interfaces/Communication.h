#pragma once
#include "../../stdafx.h"
#include "Logs.h"

class COM
{
public:

	static void RegModul(string name_, boost::function<void(string)> func_);
	static void SendModul(string name_, string mess_);

private:

	static map<string, boost::function<void(string)>> mapFunc;

};

