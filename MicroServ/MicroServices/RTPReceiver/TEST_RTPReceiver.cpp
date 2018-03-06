#include "stdafx.h"
#include "TEST_RTPReceiver.h"
using namespace RTPReceiver;

TO::TO(string c_port_, string s_port_, string id_, SHP_IO io_)
{
	mapData["CallID"] = id_;
	mapData["ClientIP"] = CFG::Param("OuterIP");
	mapData["ServerIP"] = CFG::Param("OuterIP");
	mapData["ClientPort"] = c_port_;
	mapData["ServerPort"] = s_port_;
	mapData["From"] = "TESTCLASS";
	mapData["To"] = "Receiver";

	thisSocket.reset(new Socket(
		CFG::Param("OuterIP"), c_port_,
		CFG::Param("OuterIP"), s_port_, io_
		));

	string buf("1", 172);
	thisBuf.reset(new SockBuf((uint8_t*)buf.c_str(), 172));
}
string TO::Param(string name_)
{
	return mapData[name_];
}
SHP_IPL TO::GetBasicIPL()
{
	SHP_IPL ipl;
	map<string, string> map_data;

	map_data.insert({ "To", mapData["To"] });
	map_data.insert({ "From", mapData["From"] });
	map_data.insert({ "CallID", mapData["CallID"] });
	map_data.insert({ "ClientIP", mapData["ServerIP"] });
	map_data.insert({ "ServerIP", mapData["ClientIP"] });
	map_data.insert({ "ClientPort", mapData["ServerPort"] });
	map_data.insert({ "ServerPort", mapData["ClientPort"] });

	ipl.reset(new IPL(map_data));
	return ipl;
}
void TO::Send()
{
	thisSocket->SendTo(thisBuf);
}
void TO::Func1(boost::any param_)
{
	this_thread::sleep_for(chrono::milliseconds(2));
	//cout << "Func1\n";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_thread TESTRECEIVER::sendThread;
SHP_IO TESTRECEIVER::toIO;
map<int, SHP_TO> TESTRECEIVER::mapTO;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTRECEIVER::Init(int size_)
{
	cout << "\nTEST INIT";
	toIO.reset(new IO());
	int port = 20000;
	for (int i = 0; i < size_; ++i)
	{
		SHP_TO new_to; new_to.reset(new TO(to_string(port), to_string(port + 1), to_string(i), toIO));
		mapTO[i] = new_to;
		port += 2;
	}
	sendThread.reset(new thread(&TESTRECEIVER::SendAll));
	cout << "\nTEST INIT done";
}
void TESTRECEIVER::Start(int no_requests_)
{
	for (int i = 0; i < no_requests_; ++i)
	{
		std::default_random_engine generator((unsigned)(chrono::system_clock::now().time_since_epoch().count()));
		std::uniform_int_distribution<int> distribution_user(0, 1000);
		int dice_user = distribution_user(generator);

		cout << "\nRequest: " + to_string(i);
		this_thread::sleep_for(chrono::milliseconds(10));
		if ((i+dice_user) % 10 == 0)
		{
			for (auto&e : mapTO)
			{
				this_thread::sleep_for(chrono::milliseconds(5));
				SHP_IPL ipl = e.second->GetBasicIPL();
				ipl->SetParam("CMD", "AddCaller");
				COM::SendModul("Receiver", ipl);
			}
		}
		if ((i + dice_user) % 10 == 1)
		{
			for (auto&e : mapTO)
			{
				this_thread::sleep_for(chrono::milliseconds(5));
				SHP_IPL ipl = e.second->GetBasicIPL();
				ipl->SetParam("CMD", "AddFunc");
				ipl->SetFunc("func1", boost::bind(&TO::Func1, e.second, _1));
				COM::SendModul("Receiver", ipl);
			}
		}
		if ((i + dice_user) % 10 == 2)
		{
			for (auto&e : mapTO)
			{
				this_thread::sleep_for(chrono::milliseconds(5));
				SHP_IPL ipl = e.second->GetBasicIPL();
				ipl->SetParam("CMD", "DelFunc");
				ipl->SetParam("FuncName", "func1");
				COM::SendModul("Receiver", ipl);
			}
		}
		if (i % 10 == 9)
		{
			for (auto&e : mapTO)
			{
				this_thread::sleep_for(chrono::milliseconds(5));
				SHP_IPL ipl = e.second->GetBasicIPL();
				ipl->SetParam("CMD", "DelCaller");
				COM::SendModul("Receiver", ipl);
			}
		}
	}
}
void TESTRECEIVER::SendAll()
{
	while (true)
	{
		this_thread::sleep_for(chrono::milliseconds(10));
		for (auto& e : mapTO)
		{
			e.second->Send();
		}
	}
}