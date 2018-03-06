#include "stdafx.h"
#include "Server.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
InnerServer::InnerServer()
{
	BOOST_LOG_SEV(lg, trace) << "InnerServer::InnerServer() call to iplManagement = make_shared<Control>();";
	iplManagement = make_shared<Control>();
	BOOST_LOG_SEV(lg, trace) << "InnerServer::InnerServer(): DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void InnerServer::Run()
{
	BOOST_LOG_SEV(lg, trace) << "InnerServer::Run()";
	net_Data->GS(NETDATA::in)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&InnerServer::Receive, this, _1, _2));
	BOOST_LOG_SEV(lg, trace) << "InnerServer::Run() DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void InnerServer::Receive(boost::system::error_code ec_, size_t szPack_)
{
	if (szPack_ > 10)
	{
		message.data[szPack_] = 0;
		BOOST_LOG_SEV(lg, info) << "InnerServer::Receive(...): Message received:\n" << message.data;
		SHP_IPL ipl = make_shared<IPL>(IPL(message.data, message.sender));
		BOOST_LOG_SEV(lg, debug) <<"Parsed as:\n"<< ipl->PrintAll();
		if (ipl->error == "wrong event type") { ipl->error == ""; }
		else if (ipl->error != "") ReplyError(ipl);
		else iplManagement->Preprocessing(ipl);
	}
	net_Data->GS(NETDATA::in)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&InnerServer::Receive, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void InnerServer::ReplyError(SHP_IPL ipl_)
{
	string result = ipl_->ResponseBAD(400, "\nNOT IPL. REASON:\n" + ipl_->error);
	BOOST_LOG_SEV(lg, debug) << "InnerServer::ReplyError():\n" << result;
	net_Data->GS(NETDATA::in)->s.send_to(boost::asio::buffer(result), ipl_->sender);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------