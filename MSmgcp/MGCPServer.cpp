#include "stdafx.h"
#include "MGCPServer.h"
using namespace mgcp;


MGCPServer::MGCPServer()
{
	
	mgcpManagement = make_shared<MGCPcontrol>(MGCPcontrol());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCPServer::Run()
{
	
	NET::GS(NET::mgcp)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&mgcp::MGCPServer::Receive, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCPServer::Receive(boost::system::error_code ec_, size_t size_)
{
	
	if (size_ > 10)
	{
		message.data[size_] = 0;
		
		SHP_MGCP mgcp = make_shared<MGCP>(MGCP(message.data, message.sender));
		
		if (mgcp->outerError != ""){ mgcp->ReplyNOTMGCP(); }
		else{ mgcpManagement->Preprocessing(mgcp); }
	}
	NET::GS(NET::mgcp)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&mgcp::MGCPServer::Receive, this, _1, _2));
}
