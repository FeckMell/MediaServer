#include "stdafx.h"
#include "NetData.h"
vector<SHP_SOCK> NET::socketsIN = {};
vector<SHP_SOCK> NET::socketsOUT = {};
vector<EP> NET::endPointsIN = {};
vector<SHP_IO> NET::iosIN = {};
vector<SHP_IO> NET::iosOUT = {};

void NET::Init()
{
	/* IO init */
	iosIN.resize(maxIN);
	for (int i = 0; i < maxIN; ++i) iosIN[i].reset(new IO());

	iosOUT.resize(maxOUT);
	for (int i = 0; i < maxOUT; ++i) iosOUT[i].reset(new IO());

	/* EndPoints init */
	endPointsIN.resize(maxIN);
	for (int i = 0; i < maxIN; ++i)
	{
		endPointsIN[i] = EP(
			boost::asio::ip::address::from_string(CFG::data[CFG::innerIP]),
			stoi(CFG::data[CFG::innerPort]) + i - 1
			);
	}

	/* Sockets init */
	socketsIN.resize(maxIN);
	for (int i = 0; i < maxIN; ++i)
	{
		socketsIN[i].reset(new SOCK(
			CFG::data[CFG::innerIP],
			stoi(CFG::data[CFG::innerPort]) + i - 1,
			GI(INNER(i))
			));
	}

	socketsOUT.resize(maxOUT);
	socketsOUT[OUTER::mgcp].reset(new SOCK(
		CFG::data[CFG::outerIP],
		stoi(CFG::data[CFG::mgcpPort]),
		GI(OUTER::mgcp)
		));
	socketsOUT[OUTER::sip].reset(new SOCK(
		CFG::data[CFG::outerIP],
		stoi(CFG::data[CFG::sipPort]),
		GI(OUTER::sip)
		));
}
SHP_SOCK NET::GS(INNER s_)
{
	try { return socketsIN[s_]; }
	catch (exception& e)
	{
		cout << "\n Exception:" << e.what();
		system("pause");
		exit(-1);
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
EP NET::GE(INNER e_)
{
	try { return endPointsIN[e_]; }
	catch (exception& e)
	{
		cout << "\n Exception:" << e.what();
		system("pause");
		exit(-1);
	}
}
SHP_IO NET::GI(INNER io_)
{
	try { return iosIN[io_]; }
	catch (exception& e)
	{
		cout << "\n Exception:" << e.what();
		system("pause");
		exit(-1);
	}
}
SHP_IO NET::GI(OUTER io_)
{
	try { return iosOUT[io_]; }
	catch (exception& e)
	{
		cout << "\n Exception:" << e.what();
		system("pause");
		exit(-1);
	}
}
void NET::RunIO(INNER t_)
{
	iosIN[t_]->run();
}
void NET::RunIO(OUTER t_)
{
	iosOUT[t_]->run();
}
void NET::SendModul(INNER who_, INNER where_, string what_)
{
	GS(who_)->s.send_to(boost::asio::buffer(what_), GE(where_));
}