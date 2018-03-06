#include "stdafx.h"
#include "SL_NetData.h"

vector<SHP_SOCK> NET::socketsOUT = {};
vector<boost::signals2::signal<void(string)>> NET::vecSigsIN = {};
vector<boost::signals2::signal<void()>> NET::vecSigsOUT = {};
SHP_IO NET::outerIO;

void NET::Init()
{
	/* Sigs init */
	vecSigsIN.resize(maxIN);
	vecSigsOUT.resize(maxOUT);
	 
	/* IO init */
	outerIO.reset(new IO());
	 
	/* Sockets init */
	socketsOUT.resize(maxOUT);
	 
	socketsOUT[OUTER::mgcp_].reset(new SOCK(
		CFG::data["outerIP"],
		stoi(CFG::data["mgcpPort"]),
		outerIO
		));

	if (CFG::data["sipName"] != "")
	{
		socketsOUT[OUTER::sip_].reset(new SOCK(
			CFG::data["outerIP"],
			stoi(CFG::data["sipPort"]),
			outerIO
			));
	}
	 
}

SHP_SOCK NET::GS(OUTER s_)
{
	try { return socketsOUT[s_]; }
	catch (exception& e)
	{
		cout << "\n Exception:" << e.what();
		system("pause");
		exit(-1);
	}
}
void NET::SignalInIO(INNER where_, string what_)
{
	NET::vecSigsIN[where_](what_);
}
