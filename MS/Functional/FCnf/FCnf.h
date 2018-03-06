#pragma once
#include "../../SharedLib/All.h"
#include "FCnf_Room.h"

namespace NFCnf
{
	class FCnf
	{
	public:

		static void Init();
		static void ProceedRequest(string request_);

	private:

		static void Create(SHP_IPL ipl_);
		static void Delete(SHP_IPL ipl_);

		static map<string, SHP_Room> mapRoom;

	};
}