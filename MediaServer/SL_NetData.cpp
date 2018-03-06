#include "stdafx.h"
#include "SL_NetData.h"

vector<SHP_SOCK> NET::socketsOUT = {};
vector<boost::signals2::signal<void(string)>> NET::vecSigsIN = {};
vector<boost::signals2::signal<void(REQUEST)>> NET::vecSigsOUT = {};
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
		CFG::data[CFG::outerIP],
		stoi(CFG::data[CFG::mgcpPort]),
		outerIO
		));
	 
	socketsOUT[OUTER::sip_].reset(new SOCK(
		CFG::data[CFG::outerIP],
		stoi(CFG::data[CFG::sipPort])+1,//debug
		outerIO
		));
	 
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
