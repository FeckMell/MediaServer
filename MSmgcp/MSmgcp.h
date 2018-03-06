#pragma once
#include "stdafx.h"
#include "MGCPServer.h"//
namespace mgcp
{
//#include "MGCPServer.h"

	class MSmgcp
	{
	public:
		MSmgcp();
	private:
		SHP_MGCPServer mgcpServer;
	};
	typedef shared_ptr<MSmgcp> SHP_MSmgcp;
}