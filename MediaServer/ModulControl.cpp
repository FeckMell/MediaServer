#include "stdafx.h"
#include "ModulControl.h"

MC::MC()
{
	si.resize(maxModules);
	pi.resize(maxModules);
	RunProcess("MSmgcp.exe" + MakeCmd(), 50, mgcp);
	RunProcess("MSann.exe" + MakeCmd(), 50, ann);
	RunProcess("MScnf.exe" + MakeCmd(), 50, cnf);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool MC::RunProcess(string cmd_, DWORD wait_, int which_)
{
	si[which_].wShowWindow = 0;
	ZeroMemory(&si[which_], sizeof(si[which_]));
	ZeroMemory(&pi[which_], sizeof(pi[which_]));
	si[which_].cb = sizeof(si[which_]);
	cout << "\nDEBUG: " << cmd_ << "\n\n\n";

	if (!CreateProcessA(
		NULL,
		const_cast<LPSTR>(cmd_.c_str()),
		0,
		NULL,
		TRUE,
		NORMAL_PRIORITY_CLASS,
		NULL,
		NULL,
		&si[which_],
		&pi[which_]
		))
	{
		cout << "\nCreateProcess failed for modul=" << which_ << " [" << GetLastError() << "]\n";
		return false;
	}
	WaitForSingleObject(pi[which_].hProcess, wait_);
	return true;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MC::MakeCmd()
{
	string result = " \"" +
		init_Params->data[IPar::outerIP] + "\" \"" +
		init_Params->data[IPar::innerIP] + "\" \"" +
		init_Params->data[IPar::innerPort] + "\" \"" +
		init_Params->data[IPar::logLevel] + "\" \"" +
		init_Params->data[IPar::outerPort] + "\" \"" +
		init_Params->data[IPar::rtpPort] + "\" \"" +
		init_Params->data[IPar::maxTimeAnn] + "\" \"" +
		init_Params->data[IPar::maxTimeCnf] + "\" \"" +
		init_Params->data[IPar::maxTimePrx] + "\" \"" +
		init_Params->data[IPar::mediaPath] + "\" \"" + 
		init_Params->data[IPar::homePath] + "\"";
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------