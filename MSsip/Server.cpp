#include "stdafx.h"
#include "Server.h"
using namespace sip;


SIPServer::SIPServer()
{
	sipManagement = make_shared<SIPcontrol>(SIPcontrol());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPServer::Run()
{
	NET::GS(NET::OUTER::sip)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&SIPServer::ReceiveSIP, this, _1, _2));
	NET::GS(NET::INNER::sip_i)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&SIPServer::ReceiveIN, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPServer::ReceiveSIP(boost::system::error_code ec_, size_t size_)
{
	if (size_ > 10)
	{
		message.data[size_] = 0;
		SHP_SIP sip = make_shared<SIP>(SIP(message.data, message.sender));
		if (sip->error != "")
		{
			sip->ReplyClient(NET::GS(NET::OUTER::sip), sip->ResponseBAD());
		}
		else
		{
			sipManagement->Preprocessing(sip);
		}
	}
	NET::GS(NET::OUTER::sip)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&SIPServer::ReceiveSIP, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPServer::ReceiveIN(boost::system::error_code ec_, size_t size_)
{
	if (size_ > 10)
	{
		message.data[size_] = 0;
		
		SHP_IPL ipl = make_shared<IPL>(IPL(message.data, message.sender));
		
		sipManagement->Preprocessing(ipl);
	}
	NET::GS(NET::INNER::sip_i)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&SIPServer::ReceiveIN, this, _1, _2));
	
}