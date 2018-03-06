#include "stdafx.h"
#include "Cnf.h"
using namespace sip;


Cnf::Cnf(SHP_Point point_)
{
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::Cnf(...)";
	point_->PlayAnn("music_alaw.wav");
	vecPoints.push_back(point_);
	roomID = point_->roomID;
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::Cnf(...) roomID=_" << roomID << "_END";
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
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::AddPoint(...)";
	vecPoints.push_back(point_);
	if (state == false)
	{
		state = true;
		BOOST_LOG_SEV(LOG::vecLogs, debug) << "Cnf::AddPoint(...) state==false";
		vecPoints[0]->StopAnn();
		vecPoints[1]->StopAnn();

		string client_ip = vecPoints[0]->clientIP + " " + vecPoints[1]->clientIP;
		string client_port = vecPoints[0]->clientPort + " " + vecPoints[1]->clientPort;
		string server_port = vecPoints[0]->serverPort + " " + vecPoints[1]->serverPort;

		string result = "M7S2I6P5M\n";
		result += "From=sip\n";
		result += "To=cnf\n";
		result += "EventID=sip" + roomID + "\n";
		result += "EventType=cr\n";
		result += "ClientIP=" + client_ip + "\n";
		result += "ClientPort=" + client_port + "\n";
		result += "ServerPort=" + server_port + "\n";
		NET::SendModul(NET::INNER::sip_i, NET::INNER::cnf, result);
		//net_Data->SendModul(NETDATA::cnf, result);
	}
	else
	{
		point_->StopAnn();
		BOOST_LOG_SEV(LOG::vecLogs, debug) << "Cnf::AddPoint(...) state==true";
		string result = "M7S2I6P5M\n";
		result += "From=sip\n";
		result += "To=cnf\n";
		result += "EventID=sip" + roomID + "\n";
		result += "EventType=md\n";
		result += "ClientIP=" + point_->clientIP + "\n";
		result += "ClientPort=" + point_->clientPort + "\n";
		result += "ServerPort=" + point_->serverPort + "\n";
		NET::SendModul(NET::INNER::sip_i, NET::INNER::cnf, result);
		//net_Data->SendModul(NETDATA::cnf, result);
	}
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::AddPoint(...) END";

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
bool Cnf::RmPoint(SHP_Point point_)
{
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Cnf::RmPoint(...)";
	if (state == false)
	{
		BOOST_LOG_SEV(LOG::vecLogs, debug) << "Cnf::RmPoint(...) state==false";
		point_->StopAnn();
		return true;
	}
	else
	{
		BOOST_LOG_SEV(LOG::vecLogs, debug) << "Cnf::RmPoint(...) state==true";
		if (vecPoints.size() == 1)
		{
			BOOST_LOG_SEV(LOG::vecLogs, debug) << "Cnf::RmPoint(...) state==true, vecPoints.size() == 1";
			vecPoints[0]->StopAnn();
			vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
			return true;
		}
		else if (vecPoints.size() == 2)
		{
			BOOST_LOG_SEV(LOG::vecLogs, debug) << "Cnf::RmPoint(...) state==true, vecPoints.size() == 2";
			string result = "M7S2I6P5M\n";
			result += "From=sip\n";
			result += "To=cnf\n";
			result += "EventID=sip" + roomID + "\n";
			result += "EventType=dl\n";
			NET::SendModul(NET::INNER::sip_i, NET::INNER::cnf, result);
			//net_Data->SendModul(NETDATA::cnf, result);
			vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
			state = false;
			BOOST_LOG_SEV(LOG::vecLogs, debug) << "Cnf::RmPoint(...) state==true, vecPoints.size() == 1, vecPoints[0]->PlayAnn(...)";
			vecPoints[0]->PlayAnn("music_alaw.wav");
			return false;
		}
		else
		{
			BOOST_LOG_SEV(LOG::vecLogs, debug) << "Cnf::RmPoint(...) state==true, vecPoints.size() >= 3";
			string result = "M7S2I6P5M\n";
			result += "From=sip\n";
			result += "To=cnf\n";
			result += "EventID=sip" + roomID + "\n";
			result += "EventType=md\n";
			result += "ClientIP=" + point_->clientIP + "\n";
			result += "ClientPort=" + point_->clientPort + "\n";
			result += "ServerPort=" + point_->serverPort + "\n";
			NET::SendModul(NET::INNER::sip_i, NET::INNER::cnf, result);
			//net_Data->SendModul(NETDATA::cnf, result);
			vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
			return false;
		}
	}
	BOOST_LOG_SEV(LOG::vecLogs, debug) << "Cnf::RmPoint(...) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
