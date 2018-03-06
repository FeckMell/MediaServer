#include "stdafx.h"
#include "MSsip_Cnf.h"
using namespace sip;


Cnf::Cnf(SHP_Point point_)
{
	LOG::Log(LOG::info, "SIP", "MSSIP: Cnf created with id=" + point_->roomID);
	point_->PlayAnn("music_alaw.wav");
	vecPoints.push_back(point_);
	eventID = point_->roomID;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::AddPoint(SHP_Point point_)
{
	LOG::Log(LOG::info, "SIP", "MSSIP: Cnf Add point with id=" + point_->callID);
	vecPoints.push_back(point_);
	Process();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Cnf::RmPoint(SHP_Point point_)
{
	LOG::Log(LOG::info, "SIP", "MSSIP: Cnf DL point with id=" + point_->callID);
	vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
	Process();
	if (vecPoints.size() == 0) return true;
	else return false;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::Process()
{
	vector<SHP_Point> active_points;
	for (auto& e : vecPoints) if (e->state == Point::ready) active_points.push_back(e);

	if (state == false)
	{
		if (active_points.size() == 2)
		{
			SendCR(active_points);
			state = true;
		}
	}
	else
	{
		if (active_points.size() >= 2)
		{
			SendCR(active_points);
			state = true;
		}
		else
		{
			
			SendDL();
			active_points[0]->PlayAnn("music_alaw.wav");
			state = false;
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::SendDL()
{
	string result = "From=sip\n";
	result += "To=cnf\n";
	result += "EventID=sip" + eventID + "\n";
	result += "EventType=dl\n";
	NET::vecSigsIN[NET::INNER::cnf](result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::SendCR(vector<SHP_Point> points_)
{
	string client_ip = "";
	string client_port = "";
	string server_port = "";

	for (auto&e : points_)
	{
		client_ip += e->clientSDP->data["IP"] + " ";
		client_port += e->clientSDP->data["Port"] + " ";
		server_port += e->serverSDP->data["Port"] + " ";
		if (e->playingAnn == true){ e->StopAnn(); }
	}
	client_ip.pop_back();
	client_port.pop_back();
	server_port.pop_back();

	string result = "From=sip\n";
	result += "To=cnf\n";
	result += "EventID=sip" + eventID + "\n";
	result += "EventType=cr\n";
	result += "ClientIP=" + client_ip + "\n";
	result += "ClientPort=" + client_port + "\n";
	result += "ServerPort=" + server_port + "\n";
	NET::vecSigsIN[NET::INNER::cnf](result);
}