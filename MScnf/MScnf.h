#pragma once
#include "stdafx.h"
#include "Server.h"
namespace cnf
{
	class MScnf
	{
	public:
		MScnf();
	private:
		SHP_InnerServer innerServer;
	};
	typedef shared_ptr<MScnf> SHP_MScnf;
}