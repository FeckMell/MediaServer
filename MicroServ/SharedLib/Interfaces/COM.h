#pragma once
#include "../../stdafx.h"

class COM
{
public:

	static void RegModul(string name_, boost::function<void(boost::any)> func_);
	static void SendModul(string name_, boost::any param_);

private:

	static map<string, boost::function<void(boost::any)>> mapData;

};