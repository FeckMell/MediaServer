#include "stdafx.h"
#include "OverLoad.h"
using namespace OverLoad;

map<string, SHP_Caller> MainFrame::mapCaller;
string MainFrame::modulName = "OverLoad";
SHP_IO MainFrame::ioMainFrame;
SHP_Socket MainFrame::fakeSocket;
SHP_thread MainFrame::receiveThread;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::Init()
{
	COM::RegModul(modulName, boost::bind(&MainFrame::Proceed, _1));
	LOG::AddLogInstance(modulName);

	ioMainFrame.reset(new IO());
	fakeSocket.reset(new Socket(
		CFG::Param("OuterFakeIP"), CFG::Param("OuterFakePort"),
		CFG::Param("MyFakeIP"), CFG::Param("MyFakePort"), ioMainFrame));
	fakeSocket->AsyncReceiveFrom(boost::bind(&MainFrame::FakeReceive, _1, _2));
	receiveThread.reset(new thread([&](){ioMainFrame->run(); cout << "\nOverLoad::ThreadExit"; }));

	LOG::Log("debug", modulName, "OverLoad::MainFrame::Init Done.");
}
void MainFrame::DeInit()
{

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
/*
Nessesary IPL params for income:From To ReplyTo CMD 
Nessesary IPL params for responce: From To CMD 
*/
void MainFrame::Proceed(boost::any param_)
{
	try
	{
		SHP_IPL ipl = boost::any_cast<SHP_IPL>(param_);
		if (ipl->Param("CMD") == "AddCaller")  ioMainFrame->post(boost::bind(&MainFrame::AddCaller, ipl));
		else if (ipl->Param("CMD") == "DelCaller") ioMainFrame->post(boost::bind(&MainFrame::DelCaller, ipl));
		else LOG::Log("fatal", "Errors", "OverLoad::MainFrame::Proceed Error 1.");
	}
	catch (...){ LOG::Log("fatal", "Errors", "OverLoad::MainFrame::Proceed Error 2."); }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::AddCaller(SHP_IPL ipl_)
{
	LOG::Log("debug", modulName, "OverLoad::MainFrame::AddCaller " + ipl_->Param("CallID"));

	CheckOverLoad(ipl_->Param("ClientIP"), ipl_->Param("ClientPort"));
	SHP_Caller new_caller; new_caller.reset(new Caller(ipl_));
	mapCaller.insert({ipl_->Param("CallID"), new_caller});
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::DelCaller(SHP_IPL ipl_)
{
	LOG::Log("debug", modulName, "OverLoad::MainFrame::DelCaller " + ipl_->Param("CallID"));
	mapCaller.erase(ipl_->Param("CallID"));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::CheckOverLoad(string c_ip_, string c_port_)
{
	SHP_Caller collision = nullptr;
	for (auto& e : mapCaller)
	{
		if (c_port_ == e.second->Param("ClientPort") && c_ip_ == e.second->Param("ClientIP"))
		{
			collision = e.second;
		}
	}
	if (collision != nullptr)
	{
		LOG::Log("fatal", "Errors", "OverLoad::MainFrame::CheckOverLoad Already existed. OverLoad.");
		ReportCollision(collision);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::ReportCollision(SHP_Caller caller_)
{
	map<string, string> map_data;
	map_data.insert({"To", caller_->Param("ReplyTo")});
	map_data.insert({ "From", "OverLoad" });
	map_data.insert({ "CMD", "DelCaller" });
	map_data.insert({ "CallID", caller_->Param("CallID") });
	SHP_IPL ipl; ipl.reset(new IPL(map_data));
	COM::SendModul(caller_->Param("ReplyTo"), ipl);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::FakeReceive(boost::system::error_code ec_, size_t size_)
{
	if (!ec_)
	{
		LOG::Log("fatal", "Errors", "OverLoad::MainFrame::FakeReceive received");
		fakeSocket->AsyncReceiveFrom(boost::bind(&MainFrame::FakeReceive, _1, _2));
	}
	else
	{
		LOG::Log("fatal", "Errors", "OverLoad::MainFrame::FakeReceive else");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------