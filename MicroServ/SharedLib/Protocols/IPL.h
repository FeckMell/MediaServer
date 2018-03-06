#pragma once
#include "../../stdafx.h"

class IPL
{
public:

	IPL();
	IPL(map<string, string> map_data_);
	string PrintAll();

	string Param(string name_);
	void SetParam(string name_, string value_);

	boost::function<void(boost::any)> GetFunc(string name_);
	void SetFunc(string name_, boost::function<void(boost::any)> func_);

	boost::any GetData(string name_);
	void SetData(string name_, boost::any value_);

private:

	map<string, string> mapData;
	map<string, boost::function<void(boost::any)>> mapFunc;
	map<string, boost::any> mapAny;

}; typedef shared_ptr<IPL> SHP_IPL;