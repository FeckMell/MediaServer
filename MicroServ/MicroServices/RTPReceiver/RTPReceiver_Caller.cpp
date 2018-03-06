#include "stdafx.h"
#include "RTPReceiver_Caller.h"
using namespace RTPReceiver;

Caller::Caller(SHP_IPL ipl_, SHP_IO io_)
{
	LOG::Log("debug", "RTPReceiver", "RTPReceiver::Caller::Caller _" + ipl_->Param("CallID") + "_");
	mapData.insert({ "CallID",     ipl_->Param("CallID")     });
	mapData.insert({ "ClientIP",   ipl_->Param("ClientIP")   });
	mapData.insert({ "ClientPort", ipl_->Param("ClientPort") });
	mapData.insert({ "ServerIP",   ipl_->Param("ServerIP")   });
	mapData.insert({ "ServerPort", ipl_->Param("ServerPort") });
	mapData.insert({ "SafeDel",    "0"                       });

	thisSocket.reset(new Socket(
		Param("ClientIP"), Param("ClientPort"),
		Param("ServerIP"), Param("ServerPort"), io_
		));
	thisSocket->AsyncReceiveFrom(boost::bind(&Caller::Receive, this, _1, _2));
}
void Caller::TestSafe(SHP_Caller c_)
{
	thisCaller = c_;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Caller::~Caller()
{

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::Stop()
{
	LOG::Log("debug", "RTPReceiver", "RTPReceiver::Caller::Stop _" + Param("CallID") + "_");
	thisSocket->Stop();
	mapData["SafeDel"] = "1";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::AddFunc(SHP_IPL ipl_)
{
	if (mapFunc.find(ipl_->Param("FuncName")) == mapFunc.end())
	{
		LOG::Log("debug", "RTPReceiver", "RTPReceiver::Caller::AddFunc _" + Param("FuncName") + "_");
		mapFunc.insert({ ipl_->Param("FuncName"), ipl_->GetFunc(ipl_->Param("FuncName")) });
	}
	else
	{
		LOG::Log("fatal", "Errors", "RTPReceiver::Caller::AddFunc else _" + Param("FuncName") + "_");
		mapFunc.erase(ipl_->Param("FuncName"));
		mapFunc.insert({ ipl_->Param("FuncName"), ipl_->GetFunc(ipl_->Param("FuncName")) });
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::DelFunc(SHP_IPL ipl_)
{
	LOG::Log("debug", "RTPReceiver", "RTPReceiver::Caller::DelFunc _" + Param("FuncName") + "_");
	mapFunc.erase(ipl_->Param("FuncName"));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Caller::Param(string name_)
{
	return mapData[name_];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::Receive(boost::system::error_code ec_, size_t size_)
{
	if (!ec_ && mapData["SafeDel"]!="1")
	{
		SHP_SockBuf new_data; new_data.reset(new SockBuf(thisSocket->Data(), size_));
		vector<string> badFunc;
		for (auto& func : mapFunc)
		{
			try
			{
				func.second(new_data);
			}
			catch (...)
			{
				LOG::Log("info2", "RTPReceiver", "RTPReceiver::Caller::Receive catch _" + func.first + "_");
				badFunc.push_back(func.first);
			}
		}
		for (auto& e : badFunc)
		{
			mapFunc.erase(e);
		}
		thisSocket->AsyncReceiveFrom(boost::bind(&Caller::Receive, this, _1, _2));
	}
	else
	{
		LOG::Log("debug", "RTPReceiver", "RTPReceiver::Caller::Receive else");
		thisCaller = nullptr;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------