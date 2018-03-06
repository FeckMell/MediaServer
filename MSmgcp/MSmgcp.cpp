#include "stdafx.h"
#include "MSmgcp.h"

mgcp::MSmgcp::MSmgcp()
{
	mgcpServer.reset(new MGCPServer());
	mgcpServer->Run();
	std::thread th([] { NET::RunIO(NET::OUTER::mgcp); });
	th.detach();
}
