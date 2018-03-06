#include "stdafx.h"
#include "SL_IPL.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
IPL::IPL(const char* rawMes_, EP sender_) : ipl(rawMes_), sender(sender_)
{
	ParseMain();
}
IPL::IPL(string mess_) : ipl(mess_)
{
	ParseMain();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void IPL::ParseMain()
{
	cmatch results;
	regex reg3(R"((\w+)=(.+))");
	int line_num = 0;
	string line = copy_single_n_line(ipl, line_num);
	while (line != "")
	{
		regex_match(line.c_str(), results, reg3);
		params.insert({ line_num, results.str(1) });
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