#pragma once
#include "stdafx.h"
#include "Server.h"
namespace dtmf
{
	class MSdtmf
	{
	public:
		MSdtmf();
	private:
		SHP_InnerServer innerServer;
	};
	typedef shared_ptr<MSdtmf> SHP_MSdtmf;
}