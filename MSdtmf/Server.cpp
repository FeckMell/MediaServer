#include "stdafx.h"
#include "Server.h"
using namespace dtmf;


InnerServer::InnerServer()
{
	
	iplManagement = make_shared<Control>();
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void InnerServer::Run()
{
	
	NET::GS(NET::INNER::dtmf)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&InnerServer::Receive, this, _1, _2));
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void InnerServer::Receive(boost::system::error_code ec_, size_t szPack_)
{
	if (szPack_ > 10)
	{
		message.data[szPack_] = 0;
		
		SHP_IPL ipl = make_shared<IPL>(IPL(message.data, message.sender));
		
		if (ipl->error != "")
		{
			;//TODO
		}
		else iplManagement->Preprocessing(ipl);
	}
	NET::GS(NET::INNER::dtmf)->s.async_receive_from(boost::asio::buffer(message.data, 2048), message.sender,
		boost::bind(&InnerServer::Receive, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------