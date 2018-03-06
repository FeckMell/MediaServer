#include "stdafx.h"
#include "SL_Communications.h"

vector<SHP_SOCK> COM::socketsOUT = {};
vector<boost::signals2::signal<void(string)>> COM::vecSigsIN = {};
vector<boost::signals2::signal<void()>> COM::vecSigsOUT = {};
SHP_IO COM::outerIO;

void COM::Init()
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

SHP_SOCK COM::GS(OUTER s_)
{
	try { return socketsOUT[s_]; }
	catch (exception& e)
	{
		cout << "\n Exception:" << e.what();
		system("pause");
		exit(-1);
	}
}
void COM::SignalInIO(INNER where_, string what_)
{
	COM::vecSigsIN[where_](what_);
}
