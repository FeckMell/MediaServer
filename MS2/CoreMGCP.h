#pragma once
#include "SL_All.h"

class CoreMGCP
{
public:

	CoreMGCP(); /*create submodules*/
	map<string, boost::any> RegisterMePlease();

	void ProceedMGCP(/*params*/);
	void ProceedIPL(/*params*/);
	void ProceedCTR(/*params*/);

private:

	void Receive(/*params*/);

	SHP_SOCKET _socket;

	map<string, map<string, boost::any>> _modules;

	static string _modulname;
};
string CoreMGCP::_modulname = "MGCP";