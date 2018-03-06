#pragma once
#include "../../stdafx.h"
#include "../../SharedLib.h"
namespace OverLoad
{
	class Caller
	{
	public:

		Caller(SHP_IPL ipl_);
		string Param(string name_);

	private:

		map<string, string> mapData;

	}; typedef shared_ptr<Caller> SHP_Caller;

}