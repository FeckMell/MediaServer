#include "stdafx.h"
#include "Receiver.h"
using namespace Receiver;
SHP_IO MainFrame::ioForAll; /*Caller Receive Thread*/
SHP_Socket MainFrame::fakeSocket;
SHP_thread MainFrame::receiveThread;
map<string, SHP_Inst> MainFrame::mapInst;
string MainFrame::modulName = "Receiver";
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
	receiveThread.reset(new thread([&](){ioForAll->run(); cout << "\nReceiver::ThreadExit"; }));

	LOG::Log("debug", modulName, "Receiver::MainFrame::Init Done.");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::DeInit()
{

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::Proceed(boost::any param_)
{
	try
	{
		SHP_IPL ipl = boost::any_cast<SHP_IPL>(param_);
		if (ipl->Param("CMD") == "AddReceiver")      ioForAll->post(boost::bind(&MainFrame::AddReceiver, ipl));
		else if (ipl->Param("CMD") == "DelReceiver") ioForAll->post(boost::bind(&MainFrame::DelReceiver, ipl));
		else LOG::Log("fatal", "Errors", "Receiver::MainFrame::Proceed Error 1.");
	}
	catch (...){ LOG::Log("fatal", "Errors", "Receiver::MainFrame::Proceed Error 2."); }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::AddReceiver(SHP_IPL ipl_)
{
	if (mapInst.find(ipl_->Param("ModulName")) == mapInst.end())
	{
		LOG::Log("debug", modulName, "Receiver::MainFrame::AddReceiver " + ipl_->Param("Modulname"));
		SHP_Inst new_inst; new_inst.reset(new Inst(ipl_, ioForAll));
		mapInst.insert({ ipl_->Param("ModulName"), new_inst });
	}
	else
	{
		LOG::Log("fatal", "Errors", "Receiver::MainFrame::AddReceiver " + ipl_->Param("ModulName") + " Already existed. OverLoad.");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::DelReceiver(SHP_IPL ipl_)
{
	if (mapInst.find(ipl_->Param("ModulName")) != mapInst.end())
	{
		LOG::Log("debug", modulName, "Receiver::MainFrame::DelReceiver " + ipl_->Param("ModulName"));
		mapInst.erase(ipl_->Param("ModulName"));
	}
	else
	{
		LOG::Log("fatal", "Errors", "Receiver::MainFrame::DelReceiver " + ipl_->Param("ModulName") + " not found.");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::FakeReceive(boost::system::error_code ec_, size_t size_)
{
	if (!ec_)
	{
		LOG::Log("fatal", "Errors", "Receiver::MainFrame::FakeReceive received");
		fakeSocket->AsyncReceiveFrom(boost::bind(&MainFrame::FakeReceive, _1, _2));
	}
	else
	{
		LOG::Log("fatal", "Errors", "Receiver::MainFrame::FakeReceive else");
	}
}