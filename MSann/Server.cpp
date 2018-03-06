#include "stdafx.h"
#include "Server.h"
using namespace ann;


InnerServer::InnerServer()
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "InnerServer::InnerServer() call to iplManagement = make_shared<Control>();";
	iplManagement = make_shared<Control>();
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "InnerServer::InnerServer(): DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void InnerServer::Run()
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "InnerServer::Run()";
	NET::GS(NET::INNER::ann)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&InnerServer::Receive, this, _1, _2));
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "InnerServer::Run() DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void InnerServer::Receive(boost::system::error_code ec_, size_t szPack_)
{
	if (szPack_ > 10)
	{
		message.data[szPack_] = 0;
		BOOST_LOG_SEV(LOG::GL(LOG::L::ann), info) << "InnerServer::Receive(...): Message received:\n" << message.data;
		SHP_IPL ipl = make_shared<IPL>(IPL(message.data, message.sender));
		BOOST_LOG_SEV(LOG::GL(LOG::L::ann), debug) <<"Parsed as:\n"<< ipl->PrintAll();
		if (ipl->error != "")
		{
			;//TODO
		}
		else iplManagement->Preprocessing(ipl);
	}
	NET::GS(NET::INNER::ann)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&InnerServer::Receive, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------