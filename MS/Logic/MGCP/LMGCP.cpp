#include "stdafx.h"
#include "LMGCP.h"
using namespace NLmgcp;
SHP_IO LMGCP::ioMGCP = nullptr;
SHP_Socket LMGCP::mgcpSocket = nullptr;
SHP_thread LMGCP::actionThread = nullptr;
map<string, boost::function<void(boost::any)>> LMGCP::mapModulFunc = {};

void LMGCP::Init()
{
	ioMGCP.reset(new IO());
	mgcpSocket.reset(new Socket(CFG::Param("OuterIP"), CFG::Param("MGCPPort"), ioMGCP));
	mgcpSocket->AsyncReceiveFrom(boost::bind(&LMGCP::MGCPReceive, _1, _2));
	//
	//TODO init submoduls
	NLmgcpAnn::LAnn::Init(mapModulFunc);
	NLmgcpCnf::LCnf::Init(mapModulFunc);
	//
	COM::RegModul("mgcp", boost::function<void(string)>(&LMGCP::ProceedRequest, _1));
	actionThread.reset(new thread([&](){ioMGCP->run(); cout << "\nThreadExit"; }));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LMGCP::ProceedRequest(string request_)
{
	SHP_IPL ipl; ipl.reset(new IPL(request_));
	if (ipl->Param("Error") == "NOT IPL")
	{
		SHP_MGCP mgcp; mgcp.reset(new MGCP(request_, mgcpSocket->GetEndPoint()));
		if (mgcp->Param("Error") == "")
		{
			try
			{
				mapModulFunc[mgcp->Param("EventType")](mgcp);
			}catch (...){}
		}
		mgcpSocket->SendTo(mgcp->ReplyClient2(), mgcpSocket->GetEndPoint());
	}
	else
	{
		try
		{
			mapModulFunc[ipl->Param("From")](ipl);
		}catch (...){}
	}
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LMGCP::MGCPReceive(boost::system::error_code ec_, size_t size_)
{
	SHP_SockBuf received; received.reset(new SockBuf(mgcpSocket->Buffer(), size_));
	string request((char*)received->Data());
	ProceedRequest(request);
	mgcpSocket->AsyncReceiveFrom(boost::bind(&LMGCP::MGCPReceive, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
