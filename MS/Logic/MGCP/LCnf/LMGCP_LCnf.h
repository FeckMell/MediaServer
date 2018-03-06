#pragma once
#include "../../../SharedLib/All.h"
#include "LMGCP_LCnfRoom.h"
namespace NLmgcpCnf
{
	class LCnf
	{
	public:

		static void Init(map<string, boost::function<void(boost::any)>>& map_func_);
		static void Proceed(boost::any mess_);

	private:

		static void ProceedIPL(SHP_IPL ipl_);
		static void ProceedMGCP(SHP_MGCP mgcp_);

		static void CRCX(SHP_MGCP mgcp_);
		static void MDCX(SHP_MGCP mgcp_);
		static void DLCX(SHP_MGCP mgcp_);

		static string ReserveEventID();
		static void FreeEventID(string event_id_);

		static map<string, SHP_Room> mapRoom;
		static vector<int> vecEventID;

	};
}