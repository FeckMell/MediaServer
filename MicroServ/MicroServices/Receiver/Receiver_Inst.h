#pragma once
#include "../../SharedLib.h"

namespace Receiver
{
	class Inst
	{
	public:

		Inst(SHP_IPL ipl_, SHP_IO io_);
		string Param(string name_);

	private:

		void Receive(boost::system::error_code ec_, size_t size_);

		map<string, string> mapData;
		SHP_Socket socketThis;
		boost::function<void(boost::any)> funcAction;

	}; typedef shared_ptr<Inst> SHP_Inst;
}