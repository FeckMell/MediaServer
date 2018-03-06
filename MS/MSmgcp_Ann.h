#pragma once
#include "SL_ALL.h"

namespace mgcp
{
	class Ann
	{
	public:

		Ann(SHP_Point, SHP_MGCP);

		void RequestMusic(SHP_MGCP);
		void SendNTFY();
		void Stop();

		string eventID;

	private:

		bool CheckFileExistance();
		void SendToAnnModul(string);

		bool state = false;
		string loop;
		string fileName;
		SHP_Point point;
		SHP_MGCP mgcp;
	};
	typedef shared_ptr<Ann> SHP_Ann;
}