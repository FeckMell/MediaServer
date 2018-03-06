#include "stdafx.h"
#include "IPL.h"

string IPL::Generate(map<string, string> data_)
{
	string result = "IPL\n";
	for (auto& e : data_) result += e.first + "=" + e.second + "\n";
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
IPL::IPL(string mess_)
{
	if (copy_single_n_line(mess_, 0) != "IPL")
	{
		mapData["Error"] = "NOT IPL";
		return;
	}
	mapData["IPL"] = mess_;
	ParseMain();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void IPL::ParseMain()
{
	cmatch results;
	regex reg3(R"((\w+)=(.+))");
	int line_num = 1;
	string line = copy_single_n_line(Param("IPL"), line_num);
	while (line != "")
	{
		regex_match(line.c_str(), results, reg3);
		//params.insert({ line_num, results.str(1) });
		mapData.insert({ results.str(1), results.str(2) });
		line_num++;
		line = copy_single_n_line(Param("IPL"), line_num);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string IPL::PrintAll()
{
	string result = "";
	for (auto& e : mapData)
	{
		result += "\n_" + e.first + "_=_" + e.second + "_";
	}
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string IPL::Param(string name_)
{
	return mapData[name_];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------