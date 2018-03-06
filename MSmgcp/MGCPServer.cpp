#include "stdafx.h"
#include "MGCPServer.h"

MGCPServer::MGCPServer()
{
	BOOST_LOG_SEV(lg, trace) << "MGCPServer::MGCPServer()";
	mgcpManagement = make_shared<MGCPcontrol>(MGCPcontrol());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCPServer::Run()
{
	BOOST_LOG_SEV(lg, trace) << "MGCPServer::Run()";
	net_Data->GS(NETDATA::out)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&MGCPServer::Receive, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCPServer::Receive(boost::system::error_code ec_, size_t size_)
{
	BOOST_LOG_SEV(lg, trace) << "MGCPServer::Run()";
	if (size_ > 10)
	{
		message.data[size_] = 0;
		BOOST_LOG_SEV(lg, warning) << "\nIncome Message is:\n" << message.data;
		SHP_MGCP mgcp = make_shared<MGCP>(MGCP(message.data, message.sender));
		BOOST_LOG_SEV(lg, debug) << mgcp->PrintAll();
		if (mgcp->outerError != ""){ mgcp->ReplyNOTMGCP(); }
		else{ mgcpManagement->Preprocessing(mgcp); }
	}
	net_Data->GS(NETDATA::out)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&MGCPServer::Receive, this, _1, _2));
}
