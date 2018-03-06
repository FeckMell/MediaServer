#pragma once
#include "stdafx.h"
#include "Server.h"
namespace ann
{
//#include "Server.h"

	class MSann
	{
	public:
		MSann();
	private:
		SHP_InnerServer innerServer;
	};
	typedef shared_ptr<MSann> SHP_MSann;
}