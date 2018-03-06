#pragma once
#include "stdafx.h"
#include "Control.h"
namespace ann
{
	class InnerServer
	{
	public:
		InnerServer();
		void Run();
		void Receive(boost::system::error_code, size_t);
	private:
		//void ReplyError(SHP_IPL);

		SHP_Control iplManagement;
		REQUEST message = (REQUEST());
	};
	typedef shared_ptr<InnerServer> SHP_InnerServer;
}