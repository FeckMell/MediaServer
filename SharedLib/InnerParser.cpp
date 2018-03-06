#include "stdafx.h"
#include "InnerParser.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
IPL::IPL(const char* rawMes_, EP sender_) : ipl(rawMes_), sender(sender_)
{
	ParseMain();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void IPL::ParseMain()
{
	if (copy_single_n_line(ipl, 0) != "M7S2I6P5M")
	{
		error = "NOT IPL";
		return;
	}
	cmatch results;
	regex reg3(R"((\w+)=(.+))");
	int line_num = 1;
	string line = copy_single_n_line(ipl, line_num);
	while (line != "")
	{
		regex_match(line.c_str(), results, reg3);
		params.insert({ line_num - 1, results.str(1) });
		data.insert({ results.str(1), results.str(2) });
		line_num++;
		line = copy_single_n_line(ipl, line_num);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string IPL::PrintAll()
{
	string result = "";
	for (int i = 0; i < (int)params.size(); ++i)
		result += "\nParamName=" + params[i] + "_ ParamValue=" + data[params[i]] + "_";
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------