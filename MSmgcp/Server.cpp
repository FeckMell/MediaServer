#include "stdafx.h"
#include "Server.h"


//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
MGCPServer::MGCPServer()
{
	mgcpManagement = make_shared<MGCPcontrol>(MGCPcontrol());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCPServer::Run()
{
	net_Data->GS(NETDATA::out)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&MGCPServer::Receive, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCPServer::Receive(boost::system::error_code ec_, size_t size_)
{
	cout << "\nMessage received, starting listening again and proceed this.";
	if (size_ > 10)
	{
		message.data[size_] = 0;
		cout << "\nIncome Message is:\n" << message.data<<"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
		SHP_MGCP mgcp = make_shared<MGCP>(MGCP(message.data, message.sender));
		//cout << mgcp->PrintAll();//DEBUG
		if (mgcp->error != "") ReplyError(mgcp);
		else mgcpManagement->Preprocessing(mgcp);
	}
	net_Data->GS(NETDATA::out)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&MGCPServer::Receive, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCPServer::ReplyError(SHP_MGCP mgcp_)
{
	cout << "\nReply is:\n" + mgcp_->error + "_\n=======================================================================";//DEBUG
	string result = mgcp_->ResponseBAD(400, "\nNOT MGCP. REASON:\n"+mgcp_->error);
	net_Data->GS(NETDATA::out)->s.send_to(boost::asio::buffer(result), mgcp_->sender);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------