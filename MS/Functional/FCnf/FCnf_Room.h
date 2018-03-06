#pragma once
#include "../../SharedLib/All.h"
#include "FCnf_Mixer.h"
#include "FCnf_Point.h"

namespace NFCnf
{
	class Room
	{
	public:
		Room(SHP_IPL ipl_);
		~Room();

	private:

		void GetPoints(SHP_IPL ipl_);

		vector<SHP_Point> vecPoint;
		SHP_Mixer shpMixer;

	};
	typedef shared_ptr<Room> SHP_Room;
}