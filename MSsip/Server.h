#pragma once
#include "stdafx.h"
#include "SIPparser.h"
#include "SIPcontrol.h"
namespace sip
{
	class SIPServer
	{
	public:
		SIPServer();
		void Run();
		void ReceiveSIP(boost::system::error_code, size_t);
		void ReceiveIN(boost::system::error_code, size_t);
	private:
		//SHP_IPLcontrol iplManagement;
		SHP_SIPcontrol sipManagement;
		REQUEST message = (REQUEST());
	};
	typedef shared_ptr<SIPServer> SHP_SIPServer;
}