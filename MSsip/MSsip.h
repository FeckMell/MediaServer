#pragma once
#include "stdafx.h"
#include "Server.h"
namespace sip
{
	class MSsip
	{
	public:
		MSsip();
	private:
		SHP_SIPServer sipServer;
	};
	typedef shared_ptr<MSsip> SHP_MSsip;
}