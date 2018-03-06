#include "stdafx.h"
#include "RTPReceiver.h"
using namespace RTPReceiver;

SHP_IO MainFrame::ioForAll; 
SHP_Socket MainFrame::fakeSocket;
SHP_thread MainFrame::receiveThread;
map<string, SHP_Caller> MainFrame::mapCaller;
string MainFrame::modulName = "RTPReceiver";
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::Init()
{
	COM::RegModul(modulName, boost::bind(&MainFrame::Proceed, _1));
	LOG::AddLogInstance(modulName);

	ioForAll.reset(new IO());
	fakeSocket.reset(new Socket(
		CFG::Param("OuterFakeIP"), CFG::Param("OuterFakePort"),
		CFG::Param("MyFakeIP"), CFG::Param("MyFakePort"), ioForAll));
	fakeSocket->AsyncReceiveFrom(boost::bind(&MainFrame::FakeReceive, _1, _2));
	receiveThread.reset(new thread([&](){ioForAll->run(); cout << "\nRTPReceiver::ThreadExit"; }));

	LOG::Log("debug", modulName, "RTPReceiver::MainFrame::Init Done.");
}
void MainFrame::DeInit()
{

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
/*
Nessesary IPL params for income:From To CMD CallID ClientPort ClientIP ServerPort ServerIP
Nessesary IPL params for response: 
*/
void MainFrame::Proceed(boost::any param_)
{
	try
	{
		SHP_IPL ipl = boost::any_cast<SHP_IPL>(param_);
		if (ipl->Param("CMD") == "AddCaller")      ioForAll->post(boost::bind(&MainFrame::AddCaller, ipl));
		else if (ipl->Param("CMD") == "DelCaller") ioForAll->post(boost::bind(&MainFrame::DelCaller, ipl));
		else if (ipl->Param("CMD") == "AddFunc")   ioForAll->post(boost::bind(&MainFrame::AddFunc, ipl));
		else if (ipl->Param("CMD") == "DelFunc")   ioForAll->post(boost::bind(&MainFrame::DelFunc, ipl));
		else LOG::Log("fatal", "Errors", "RTPReceiver::MainFrame::Proceed Error 1.");
	}
	catch (...){ LOG::Log("fatal", "Errors", "RTPReceiver::MainFrame::Proceed Error 2."); }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::AddCaller(SHP_IPL ipl_)
{
	if (mapCaller.find(ipl_->Param("CallID")) == mapCaller.end())
	{
		LOG::Log("debug", modulName, "RTPReceiver::MainFrame::AddCaller " + ipl_->Param("CallID"));
		SHP_Caller new_caller; new_caller.reset(new Caller(ipl_, ioForAll));
		mapCaller.insert({ ipl_->Param("CallID"), new_caller });
		new_caller->TestSafe(new_caller);
	}
	else
	{
		LOG::Log("fatal", "Errors", "RTPReceiver::MainFrame::AddCaller " + ipl_->Param("CallID")+" Already existed. OverLoad.");
		mapCaller[ipl_->Param("CallID")]->Stop();
		mapCaller[ipl_->Param("CallID")].reset();
		SHP_Caller new_caller; new_caller.reset(new Caller(ipl_, ioForAll));
		mapCaller[ipl_->Param("CallID")] = new_caller;
		new_caller->TestSafe(new_caller);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::DelCaller(SHP_IPL ipl_)
{
	if (mapCaller.find(ipl_->Param("CallID")) != mapCaller.end())
	{
		LOG::Log("debug", modulName, "RTPReceiver::MainFrame::DelCaller " + ipl_->Param("CallID"));
		mapCaller[ipl_->Param("CallID")]->Stop();
		mapCaller.erase(ipl_->Param("CallID"));
	}
	else
	{
		LOG::Log("fatal", "Errors", "RTPReceiver::MainFrame::DelCaller " + ipl_->Param("CallID") + " not found.");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::AddFunc(SHP_IPL ipl_)
{
	auto found_caller = mapCaller.find(ipl_->Param("CallID"));
	if (found_caller != mapCaller.end())
	{
		LOG::Log("debug", modulName, "RTPReceiver::MainFrame::AddFunc to _" + ipl_->Param("CallID") + "_ FuncName=_" + ipl_->Param("FuncName") + "_");
		found_caller->second->AddFunc(ipl_);
	}
	else
	{
		LOG::Log("fatal", "Errors", "RTPReceiver::MainFrame::AddFunc to _" + ipl_->Param("CallID") + "_ FuncName=_" + ipl_->Param("FuncName") + "_");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::DelFunc(SHP_IPL ipl_)
{
	auto found_caller = mapCaller.find(ipl_->Param("CallID"));
	if (found_caller != mapCaller.end())
	{
		LOG::Log("debug", modulName, "RTPReceiver::MainFrame::DelFunc to _" + ipl_->Param("CallID") + "_ FuncName=_" + ipl_->Param("FuncName") + "_");
		found_caller->second->DelFunc(ipl_);
	}
	else
	{
		LOG::Log("fatal", "Errors", "RTPReceiver::MainFrame::DelFunc to _" + ipl_->Param("CallID") + "_ FuncName=_" + ipl_->Param("FuncName") + "_");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::FakeReceive(boost::system::error_code ec_, size_t size_)
{
	if (!ec_)
	{
		LOG::Log("fatal", "Errors", "RTPReceiver::MainFrame::FakeReceive received");
		fakeSocket->AsyncReceiveFrom(boost::bind(&MainFrame::FakeReceive, _1, _2));
	}
	else
	{
		LOG::Log("fatal", "Errors", "RTPReceiver::MainFrame::FakeReceive else");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------