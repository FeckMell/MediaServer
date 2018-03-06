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
	outer_Socket->s.async_receive_from(boost::asio::buffer(message.rawMes, 2048), message.sender,
		boost::bind(&MGCPServer::Receive, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCPServer::Receive(boost::system::error_code ec_, size_t szPack_)
{
	cout << "\nMessage received, starting listening again and proceed this.";
	if (szPack_ > 10)
	{
		message.rawMes[szPack_] = 0;
		cout << "\nIncome Message is:\n" << message.rawMes<<"\n+++++++++++++++++++++++++++++++++++++++++++++++++";
		SHP_MGCP mgcp = make_shared<MGCP>(MGCP(message.rawMes, message.sender));
		//cout << mgcp->PrintAll();//DEBUG
		if (mgcp->error != "") ReplyError(mgcp);
		else mgcpManagement->Preprocessing(mgcp);
	}
	outer_Socket->s.async_receive_from(boost::asio::buffer(message.rawMes, 2048), message.sender,
		boost::bind(&MGCPServer::Receive, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCPServer::ReplyError(SHP_MGCP mgcp_)
{
	cout << "\nReply is:\n" + mgcp_->error + "_\n=======================================================================";//DEBUG
	string result = mgcp_->ResponseBAD(400, "\nNOT MGCP. REASON:\n"+mgcp_->error);
	outer_Socket->s.send_to(boost::asio::buffer(result), mgcp_->sender);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------