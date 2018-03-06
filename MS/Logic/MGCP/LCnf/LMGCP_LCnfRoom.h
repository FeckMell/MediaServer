#pragma once
#include "../../../SharedLib/All.h"
#include "LMGCP_LCnfPoint.h"
namespace NLmgcpCnf
{
	class Room
	{
	public:

		Room(string event_id_);

		void AddPoint(SHP_MGCP mgcp_);
		bool RemPoint(SHP_MGCP mgcp_);
		void ModPoint(SHP_MGCP mgcp_);

		SHP_Point FindPoint(string call_id_);

		string Param(string name_);
		string PrintAll();

	private:

		void Proceed();
		void SendModul(string type_, vector<string> points_);

		map<string, string> mapData;
		map<string, SHP_Point> mapPoint;

	};
	typedef shared_ptr<Room> SHP_Room;
}