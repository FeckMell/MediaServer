#include "stdafx.h"
#include "Cnf.h"
using namespace mgcp;


Cnf::Cnf(SHP_Point point_, string event_id_)
{
	
	vecPoints.push_back(point_);
	eventID = event_id_;
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::AddPoint(SHP_Point point_)
{
	
	vecPoints.push_back(point_);
	Process(point_);
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Cnf::DeletePoint(SHP_Point point_)
{
	
	vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
	if (vecPoints.size() == 0)
	{
		
		return true;
	}
	else 
	{ 
		
		Process(point_);
		return false; 
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::Process(SHP_Point point_)
{
	
	int active_points = GetNumOfActivePoints();
	if (state == false)
	{
		
		if (active_points == 2)
		{
			
			state = true;
			
			

			string client_ip = vecPoints[0]->clientIP + " " + point_->clientIP;
			string client_port = vecPoints[0]->clientPort + " " + point_->clientPort;
			string server_port = vecPoints[0]->serverPort + " " + point_->serverPort;

			

			string result = "M7S2I6P5M\n";
			result += "From=mgcp\n";
			result += "To=cnf\n";
			result += "EventID=mgcp" + eventID + "\n";
			result += "EventType=cr\n";
			result += "ClientIP=" + client_ip + "\n";
			result += "ClientPort=" + client_port + "\n";
			result += "ServerPort=" + server_port + "\n";
			NET::SendModul(NET::INNER::mgcp_i, NET::INNER::cnf, result);
		}
	}
	else
	{
		
		if (active_points == 1)
		{
			
			state = false;

			string result = "M7S2I6P5M\n";
			result += "From=mgcp\n";
			result += "To=cnf\n";
			result += "EventID=mgcp" + eventID + "\n";
			result += "EventType=dl\n";
			NET::SendModul(NET::INNER::mgcp_i, NET::INNER::cnf, result);
		}
		else if (active_points >1 )
		{
			
			string result = "M7S2I6P5M\n";
			result += "From=mgcp\n";
			result += "To=cnf\n";
			result += "EventID=mgcp" + eventID + "\n";
			result += "EventType=md\n";
			result += "ClientIP=" + point_->clientIP + "\n";
			result += "ClientPort=" + point_->clientPort + "\n";
			result += "ServerPort=" + point_->serverPort + "\n";
			NET::SendModul(NET::INNER::mgcp_i, NET::INNER::cnf, result);
		}
		else 
		{
			
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int Cnf::GetNumOfActivePoints()
{
	int result = 0;
	for (auto point : vecPoints) if (point->state == true) result++;
	
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------