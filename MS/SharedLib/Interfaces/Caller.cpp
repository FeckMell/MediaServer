#include "stdafx.h"
#include "Caller.h"

SHP_IO CallerStore::ioForAll = nullptr;
SHP_Socket CallerStore::fakeSocket = nullptr;
SHP_thread CallerStore::receiveThread = nullptr;
map<string, SHP_Caller> CallerStore::mapCaller = {};
mutex CallerStore::thisMutex;

Caller::Caller(string call_id_)
{
	mapData["CallID"] = call_id_;
	shpTimer.reset(new Timer());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Caller::~Caller()
{
	SocketStore::FreeSocket(serverSDP->Param("Port"));
}
void Caller::StopActivity()
{
	funcMutex.lock();
	//mapData["Stop"] = "1";
	shpSocket->Cancel();
	mapFunc.clear();
	funcMutex.unlock();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Caller::Check()
{
	if (clientSDP == nullptr || serverSDP == nullptr) {cout << "\n null"; return false;}
	else if (clientSDP->Param("IP") == "" || serverSDP->Param("IP") == "") { cout << "\n IP"; return false; }
	else if (clientSDP->Param("Port") == "" || serverSDP->Param("Port") == "") { cout << "\n Port"; return false; }
	else if (clientSDP->Param("Mode") == "inactive" || serverSDP->Param("Port") == "inactive") { cout << "\n Mode"; return false; }
	return true;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::SetSDP(string type_, SHP_SDP new_sdp_)
{
	if (type_ == "Server")
	{
		if (serverSDP == nullptr)
		{
			serverSDP = new_sdp_;
			shpSocket = SocketStore::GetSocket(serverSDP->Param("Port"));

		}
		else
		{
			//TODO
			cout << "Caller::SetSDP undone 1";
			LOG::Log("fatal", "Errors", "Caller::SetSDP undone 1");
			system("pause");
			exit(-1);
		}
	}
	else if (type_ == "Client")
	{
		if (clientSDP == nullptr)
		{
			clientSDP = new_sdp_;
			shpSocket->SetEndPoint(clientSDP->Param("IP"), clientSDP->Param("Port"));
			//CallerStore::CheckOverload(this);
			shpSocket->AsyncReceiveFrom(boost::bind(&Caller::ReceiveFromSocket, this, _1, _2));
		}
		else
		{
			serverSDP->ChangeModeS(clientSDP->Param("Mode"));
		}
	}
	else
	{
		cout << "Caller::SetSDP Error 3";
		LOG::Log("fatal", "Errors", "Caller::SetSDP Error 3");
		system("pause");
		exit(-1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_SDP Caller::GetSDP(string type_)
{
	if (type_ == "Server") return serverSDP;
	else if (type_ == "Client") return clientSDP;
	else
	{
		//TODO
		cout << "\n BAD Caller::GetSDP(string type_)";
		LOG::Log("fatal", "Errors", "\n BAD Caller::GetSDP(string type_)");
		system("pause");
		exit(-1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::AddReceiveFunc(string func_name_, boost::function<void(SHP_SockBuf)> func_)
{
	funcMutex.lock();
	mapFunc[func_name_] = func_;
	funcMutex.unlock();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::RemReceiveFunc(string func_name_)
{
	funcMutex.lock();
	mapFunc.erase(func_name_);
	funcMutex.unlock();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::ReceiveFromSocket(boost::system::error_code ec_, size_t size_)
{
	if (ec_) 
	{
		LOG::Log("debug", "Shared", "\nCallerReceiveSTOP1:");
		return;
	}
	if (mapData["Stop"] == "1")
	{
		LOG::Log("fatal", "Errors", "\nCallerReceiveSTOP2:");
		return;
	}
	funcMutex.lock();
	shpTimer->Update();
	SHP_SockBuf new_data; new_data.reset(new SockBuf(shpSocket->Buffer(), size_));
	vector<string> badFunc;
	
	for (auto& func : mapFunc)
	{
		try
		{
			func.second(new_data);
		}
		catch (...)
		{
			badFunc.push_back(func.first);
		}
	}
	for (auto& e : badFunc)
	{
		mapFunc.erase(e);
	}
	shpSocket->AsyncReceiveFrom(boost::bind(&Caller::ReceiveFromSocket, this, _1, _2));
	funcMutex.unlock();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::SetParam(string name_, string value_)
{
	mapData[name_] = value_;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Caller::Param(string name_)
{
	if (name_ == "ClientIP") return clientSDP->Param("IP");
	else if (name_ == "ServerIP") return serverSDP->Param("IP");
	else if (name_ == "ClientPort") return clientSDP->Param("Port");
	else if (name_ == "ServerPort") return serverSDP->Param("Port");
	else return mapData[name_];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::SendTo(SHP_SockBuf mess_)
{
	SHP_SockBuf result; result.reset(new SockBuf(headerRTP.Get(mess_->Size() / 80), 12));
	result->AddData(mess_->Data(), mess_->Size());
	shpSocket->SendTo(result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void CallerStore::Init()
{
	ioForAll.reset(new IO());
	fakeSocket.reset(new Socket(CFG::Param("MyFakeIP"), CFG::Param("MyFakePort"), ioForAll));
	fakeSocket->SetEndPoint(CFG::Param("OuterFakeIP"), CFG::Param("OuterFakePort"));
	fakeSocket->AsyncReceiveFrom(boost::bind(&CallerStore::FakeReceive, _1, _2));

	receiveThread.reset(new thread([&](){ioForAll->run(); cout << "\nThreadExit"; }));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void CallerStore::CreateCaller(string call_id_)
{
	string server_port = SocketStore::CreateSocket(ioForAll);
	SHP_SDP server_sdp; server_sdp.reset(new SDP(call_id_, server_port));
	SHP_Caller new_caller; new_caller.reset(new Caller(call_id_));

	new_caller->SetSDP("Server", server_sdp);
	thisMutex.lock();
	mapCaller.insert({ call_id_, new_caller});
	thisMutex.unlock();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Caller CallerStore::GetCaller(string call_id_)
{
	thisMutex.lock();
	for (auto&e : mapCaller)
	{
		if (e.first == call_id_) { thisMutex.unlock(); return e.second; }
	}
	thisMutex.lock();
	return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void CallerStore::DelCaller(string call_id_)
{
	thisMutex.lock();
	mapCaller[call_id_]->StopActivity();
	mapCaller.erase(call_id_);
	thisMutex.unlock();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void CallerStore::CheckOverload(Caller* target_)
{
	string port = target_->Param("ClientPort");
	string ip = target_->Param("ClientIP");
	string call_id = target_->Param("CallID");
	cout << "\nCallID=" + call_id + "_\nIP=" + ip + "_ port=" + port + "_\n\n";
	map<string, string> collisions;

	for (auto& e : mapCaller)
	{
		if (call_id != e.second->Param("CallID"))
		{
			cout << "\nCallID2="; 
			cout << e.second->Param("CallID"); 
			cout << "_\nIP2="; 
			cout << e.second->Param("ClientIP"); 
			cout << "_ port2="; 
			cout << e.second->Param("ClientPort"); 
			cout << "_";
			if (port == e.second->Param("ClientPort") && ip == e.second->Param("ClientIP"))
			{
				string ipl = IPL::Generate({
					{ "From", e.second->Param("Type") }, { "To", e.second->Param("Creator") },
					{ "CMD", "DL" }, { "CallID", e.second->Param("CallID") }
				});
				collisions[ipl] = e.second->Param("Creator");
			}
		}
	}
	for (auto&e : collisions) COM::SendModul(e.second, e.first);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void CallerStore::FakeReceive(boost::system::error_code ec_, size_t size_)
{
	if (!ec_)
	{
		LOG::Log("fatal", "Errors", "CallerStore::FakeReceive");
		this_thread::sleep_for(chrono::milliseconds(5)); //DebugTest
		fakeSocket->AsyncReceiveFrom(boost::bind(&CallerStore::FakeReceive, _1, _2));
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------