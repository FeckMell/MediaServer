#include "stdafx.h"
#include "FCnf_Room.h"
using namespace NFCnf;

Room::Room(SHP_IPL ipl_)
{
	GetPoints(ipl_);
	shpMixer.reset(new Mixer(vecPoint));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Room::~Room()
{
	shpMixer->StopActivity();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Room::GetPoints(SHP_IPL ipl_)
{
	for (int i = 0; i < stoi(CFG::Param("RoomSize")); ++i)
	{
		string callid = ipl_->Param("CallID" + to_string(i));
		if (callid == "") break;
		SHP_Point new_point; new_point.reset(new Point(callid));
		vecPoint.push_back(new_point);
	}
	for (auto& e : vecPoint)
	{
		e->SetJitterSize(vecPoint.size());
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------