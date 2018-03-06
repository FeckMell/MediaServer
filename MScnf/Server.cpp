#include "stdafx.h"
#include "Server.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
InnerServer::InnerServer()
{
	iplManagement = make_shared<Control>();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void InnerServer::Run()
{
	inner_Socket->s.async_receive_from(boost::asio::buffer(message.rawMes, 2048), message.sender,
		boost::bind(&InnerServer::Receive, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void InnerServer::Receive(boost::system::error_code ec_, size_t szPack_)
{
	cout << "\nMessage received, starting listening again and proceed this.";
	if (szPack_ > 10)
	{
		message.rawMes[szPack_] = 0;
		cout << "\nIncome Message is:\n" << message.rawMes << "\n+++++++++++++++++++++++++++++++++++++++++++++++++";
		SHP_IPL ipl = make_shared<IPL>(IPL(message.rawMes, message.sender));
		if (ipl->error == "wrong event type") { ipl->error = ""; }
		else if (ipl->error != "") { cout << ipl->PrintAll(); ReplyError(ipl); }
		else iplManagement->Preprocessing(ipl);  
	}
	inner_Socket->s.async_receive_from(boost::asio::buffer(message.rawMes, 2048), message.sender,
		boost::bind(&InnerServer::Receive, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void InnerServer::ReplyError(SHP_IPL ipl_)
{
	cout << "\nReply is:\n" + ipl_->error + "_\n=======================================================================";//DEBUG
	string result = ipl_->ResponseBAD(400, "\nNOT IPL. REASON:\n" + ipl_->error);
	inner_Socket->s.send_to(boost::asio::buffer(result), ipl_->sender);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------