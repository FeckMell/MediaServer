#include "stdafx.h"
#include "MSdtmf.h"
using namespace dtmf;


Control::Control()
{	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Preprocessing(string message_)
{
	SHP_IPL ipl = make_shared<IPL>(IPL(message_));
	LOG::Log(LOG::info, "DTMF", "MSDTMF: ipl=\n" + message_);
	if (ipl->data["EventType"] == "cr") { CR(ipl); }
	else if (ipl->data["EventType"] == "dl") {DL(ipl);}
	else
	{
		LOG::Log(LOG::fatal, "DTMF", "MSDTMF: ipl bad:\n" + ipl->ipl);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::CR(SHP_IPL ipl_)
{
	SHP_Point new_point = make_shared<Point>(Point(ipl_));
	vecPoints.push_back(new_point);
	new_point->Run();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DL(SHP_IPL ipl_)
{
	SHP_Point found_point = FindPoint(ipl_->data["EventID"]);
	if (found_point == nullptr)
	{
		LOG::Log(LOG::fatal, "DTMF", "MSDTMF: DL: point not found id=" + ipl_->data["EventID"]);
		return;
	}
	RemovePoint(found_point);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::RemovePoint(SHP_Point point_)
{
	vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Point Control::FindPoint(string eventid_)
{
	for (auto &point : vecPoints) if (point->eventID == eventid_) return point;
	return nullptr;
}