#pragma once
#include "../../SharedLib/All.h"
#include "LAnn/LMGCP_LAnn.h"
#include "LCnf/LMGCP_LCnf.h"

namespace NLmgcp
{
	class LMGCP
	{
	public:

		static void Init();
		static void ProceedRequest(string request_);

	private:

		static void MGCPReceive(boost::system::error_code ec_, size_t size_);

		static SHP_IO ioMGCP; 
		static SHP_Socket mgcpSocket;
		static SHP_thread actionThread;

		static map<string, boost::function<void(boost::any)>> mapModulFunc;

	};
}