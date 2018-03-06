#include "stdafx.h"
#include "MSsip_Cnf.h"
using namespace sip;


Cnf::Cnf(SHP_Point point_)
{
	point_->PlayAnn("music_alaw.wav");
	vecPoints.push_back(point_);
	roomID = point_->roomID;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Point Cnf::FindPoint(string callid_)
{
	for (auto &point : vecPoints) if (point->callID == callid_) return point;
	return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::AddPoint(SHP_Point point_)
{
	
	vecPoints.push_back(point_);
	if (state == false)
	{
		state = true;
		
		vecPoints[0]->StopAnn();
		vecPoints[1]->StopAnn();

		string client_ip = vecPoints[0]->clientIP + " " + vecPoints[1]->clientIP;
		string client_port = vecPoints[0]->clientPort + " " + vecPoints[1]->clientPort;
		string server_port = vecPoints[0]->serverPort + " " + vecPoints[1]->serverPort;

		string result = "";
		result += "From=sip\n";
		result += "To=cnf\n";
		result += "EventID=sip" + roomID + "\n";
		result += "EventType=cr\n";
		result += "ClientIP=" + client_ip + "\n";
		result += "ClientPort=" + client_port + "\n";
		result += "ServerPort=" + server_port + "\n";
		NET::vecSigsIN[NET::INNER::cnf](result);
	}
	else
	{
		point_->StopAnn();
		
		string result = "";
		result += "From=sip\n";
		result += "To=cnf\n";
		result += "EventID=sip" + roomID + "\n";
		result += "EventType=md\n";
		result += "ClientIP=" + point_->clientIP + "\n";
		result += "ClientPort=" + point_->clientPort + "\n";
		result += "ServerPort=" + point_->serverPort + "\n";
		NET::vecSigsIN[NET::INNER::cnf](result);
	}
	

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Cnf::RmPoint(SHP_Point point_)
{
	
	if (state == false)
	{
		
		point_->StopAnn();
		return true;
	}
	else
	{
		
		if (vecPoints.size() == 1)
		{
			
			vecPoints[0]->StopAnn();
			vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
			return true;
		}
		else if (vecPoints.size() == 2)
		{
			
			string result = "";
			result += "From=sip\n";
			result += "To=cnf\n";
			result += "EventID=sip" + roomID + "\n";
			result += "EventType=dl\n";
			NET::vecSigsIN[NET::INNER::cnf](result);

			vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
			state = false;
			
			vecPoints[0]->PlayAnn("music_alaw.wav");
			return false;
		}
		else
		{
			
			string result = "";
			result += "From=sip\n";
			result += "To=cnf\n";
			result += "EventID=sip" + roomID + "\n";
			result += "EventType=md\n";
			result += "ClientIP=" + point_->clientIP + "\n";
			result += "ClientPort=" + point_->clientPort + "\n";
			result += "ServerPort=" + point_->serverPort + "\n";
			NET::vecSigsIN[NET::INNER::cnf](result);

			vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
			return false;
		}
	}
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
