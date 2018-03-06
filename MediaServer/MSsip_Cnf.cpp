#include "stdafx.h"
#include "MSsip_Cnf.h"
using namespace sip;


Cnf::Cnf(SHP_Point point_)
{
	point_->PlayAnn("music_alaw.wav");
	vecPoints.push_back(point_);
	eventID = point_->roomID;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::AddPoint(SHP_Point point_)
{
	vecPoints.push_back(point_);
	point_->StopAnn();
	Process();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Cnf::RmPoint(SHP_Point point_)
{
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
