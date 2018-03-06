#include "stdafx.h"
#include "Server.h"
using namespace sip;


SIPServer::SIPServer()
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPServer::SIPServer()";
	sipManagement = make_shared<SIPcontrol>(SIPcontrol());
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPServer::SIPServer() END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPServer::Run()
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPServer::Run()";
	NET::GS(NET::OUTER::sip)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&SIPServer::ReceiveSIP, this, _1, _2));
	//net_Data->GS(NETDATA::outsip)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
	//	boost::bind(&SIPServer::ReceiveSIP, this, _1, _2));
	NET::GS(NET::INNER::sip_i)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&SIPServer::ReceiveIN, this, _1, _2));
	//net_Data->GS(NETDATA::in)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		//boost::bind(&SIPServer::ReceiveIN, this, _1, _2));
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPServer::Run() END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPServer::ReceiveSIP(boost::system::error_code ec_, size_t size_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPServer::ReceiveSIP()";
	if (size_ > 10)
	{
		message.data[size_] = 0;
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), warning) << "\nIncome Message is:\n" << message.data;
		SHP_SIP sip = make_shared<SIP>(SIP(message.data, message.sender));
		if (sip->error != "")
		{
			BOOST_LOG_SEV(LOG::GL(LOG::L::sip), info) << "SIPServer::ReceiveSIP() sip->error != \"\"";
			sip->ReplyClient(NET::GS(NET::OUTER::sip), sip->ResponseBAD());
			//sip->ReplyClient(net_Data->GS(NETDATA::outsip), sip->ResponseBAD());
		}
		else
		{
			BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPServer::ReceiveSIP() ->sipManagement->Preprocessing(sip)";
			sipManagement->Preprocessing(sip);
		}
	}
	NET::GS(NET::OUTER::sip)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&SIPServer::ReceiveSIP, this, _1, _2));
	//net_Data->GS(NETDATA::outsip)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
	//	boost::bind(&SIPServer::ReceiveSIP, this, _1, _2));
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPServer::ReceiveSIP() END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPServer::ReceiveIN(boost::system::error_code ec_, size_t size_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPServer::ReceiveIN";
	if (size_ > 10)
	{
		message.data[size_] = 0;
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), debug) << "\nIncome Message is:\n" << message.data;
		SHP_IPL ipl = make_shared<IPL>(IPL(message.data, message.sender));
		BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPServer::ReceiveIN ->sipManagement->Preprocessing(ipl);";
		sipManagement->Preprocessing(ipl);
	}
	NET::GS(NET::INNER::sip_i)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&SIPServer::ReceiveIN, this, _1, _2));
	//net_Data->GS(NETDATA::in)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
	//	boost::bind(&SIPServer::ReceiveIN, this, _1, _2));
	BOOST_LOG_SEV(LOG::GL(LOG::L::sip), trace) << "SIPServer::ReceiveIN END";
}