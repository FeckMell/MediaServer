#pragma once
#include "stdafx.h"
#include "MGCPparser.h"
#include "MGCPcontrol.h"

namespace mgcp
{
	class MGCPServer
	{
	public:
		MGCPServer();
		void Run();

	private:
		void Receive(boost::system::error_code, size_t);

		SHP_MGCPcontrol mgcpManagement;
		REQUEST message = (REQUEST());
	};
	typedef shared_ptr<MGCPServer> SHP_MGCPServer;
}