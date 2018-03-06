#include "stdafx.h"
#include "Cnf.h"
using namespace mgcp;


Cnf::Cnf(SHP_Point point_, string event_id_)
{
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::Cnf with eventID=" << event_id_;
	vecPoints.push_back(point_);
	eventID = event_id_;
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::Cnf END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::AddPoint(SHP_Point point_)
{
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::AddPoint with eventID=" << eventID;
	vecPoints.push_back(point_);
	Process(point_);
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::AddPoint END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Cnf::DeletePoint(SHP_Point point_)
{
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::DeletePoint with eventID=" << eventID;
	vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
	if (vecPoints.size() == 0)
	{
		BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::DeletePoint vecPoints.size() == 0";
		return true;
	}
	else 
	{ 
		BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::DeletePoint vecPoints.size() != 0";
		Process(point_);
		return false; 
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::Process(SHP_Point point_)
{
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::Process with eventID=" << eventID;
	int active_points = GetNumOfActivePoints();
	if (state == false)
	{
		BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::Process state == false";
		if (active_points == 2)
		{
			BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::Process state == false, active_points == 2";
			state = true;
			BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::Process point 0:" + vecPoints[0]->PrintPoint();
			BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::Process point 1:" + vecPoints[1]->PrintPoint();

			string client_ip = vecPoints[0]->clientIP + " " + point_/*vecPoints[1]*/->clientIP;
			string client_port = vecPoints[0]->clientPort + " " + point_/*vecPoints[1]*/->clientPort;
			string server_port = vecPoints[0]->serverPort + " " + point_/*vecPoints[1]*/->serverPort;

			

			string result = "M7S2I6P5M\n";
			result += "From=mgcp\n";
			result += "To=cnf\n";
			result += "EventID=mgcp" + eventID + "\n";
			result += "EventType=cr\n";
			result += "ClientIP=" + client_ip + "\n";
			result += "ClientPort=" + client_port + "\n";
			result += "ServerPort=" + server_port + "\n";
			NET::SendModul(NET::INNER::mgcp_i, NET::INNER::cnf, result);
			//net_Data->SendModul(NETDATA::cnf, result);
		}
	}
	else
	{
		BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::Process state == true,";
		if (active_points == 1)
		{
			BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::Process state == true, active_points == 1";
			state = false;

			string result = "M7S2I6P5M\n";
			result += "From=mgcp\n";
			result += "To=cnf\n";
			result += "EventID=mgcp" + eventID + "\n";
			result += "EventType=dl\n";
			NET::SendModul(NET::INNER::mgcp_i, NET::INNER::cnf, result);
			//net_Data->SendModul(NETDATA::cnf, result);
		}
		else if (active_points >1 )
		{
			BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::Process state == true, active_points > 1";
			string result = "M7S2I6P5M\n";
			result += "From=mgcp\n";
			result += "To=cnf\n";
			result += "EventID=mgcp" + eventID + "\n";
			result += "EventType=md\n";
			result += "ClientIP=" + point_->clientIP + "\n";
			result += "ClientPort=" + point_->clientPort + "\n";
			result += "ServerPort=" + point_->serverPort + "\n";
			NET::SendModul(NET::INNER::mgcp_i, NET::INNER::cnf, result);
			//net_Data->SendModul(NETDATA::cnf, result);
		}
		else 
		{
			BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::Process state == true, active_points < 1";
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int Cnf::GetNumOfActivePoints()
{
	int result = 0;
	for (auto point : vecPoints) if (point->state == true) result++;
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::GetNumOfActivePoints =" << result;
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------