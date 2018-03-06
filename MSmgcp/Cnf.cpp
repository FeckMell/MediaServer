#include "stdafx.h"
#include "Cnf.h"

Cnf::Cnf(SHP_MGCP mgcp_)
{
	BOOST_LOG_SEV(lg, trace) << "Cnf::Cnf(...) eventID=" << mgcp_->data[MGCP::EventID];
	eventID = mgcp_->data[MGCP::EventID];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::CRCX(SHP_MGCP mgcp_, string server_sdp_, string server_port_)
{
	BOOST_LOG_SEV(lg, trace) << "Cnf::CRCX(...) for eventID=" << mgcp_->data[MGCP::EventID] << " creating new point";
	SHP_CallerBase new_point = make_shared<CallerBase>(CallerBase(mgcp_, server_sdp_, server_port_));
	vecCallerBase.push_back(new_point);
	BOOST_LOG_SEV(lg, error) << "Created Added point to CNF ID=" << eventID << " with params:" << "\n1)ID=" << new_point->callID << "\n2)IP=" << new_point->clientIP << "\n3)Port=" << new_point->clientPort << "\n4)ServerPort=" << new_point->serverPort;
	mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseOK(200, "add event type") + "\n\n" + server_sdp_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::MDCX(SHP_MGCP mgcp_)
{
	BOOST_LOG_SEV(lg, trace) << "Cnf::MDCX(...) for eventID=" << mgcp_->data[MGCP::EventID];
	SHP_CallerBase found_point = FindCallerBase(mgcp_);
	if (found_point==nullptr)
	{
		mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, "Error. Client could not be found.3"));
		return;
	}
	BOOST_LOG_SEV(lg, trace) << "Cnf::MDCX(...): next call: found_point->ModifyCallerBase";
	string action_result = found_point->ModifyCallerBase(mgcp_);
	if (action_result == "0") // client sent first SDP
	{
		if (state)
		{
			SendToCnfModulMD_DL(found_point);
			BOOST_LOG_SEV(lg, error) << "Point with ID=" << found_point->callID << " is now active (Sent SDP) and joined conf";
			mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseOK(200, ""));
			return;
		}
		else
		{
			state = true;
			SendToCnfModulCR(); //point state inside
			BOOST_LOG_SEV(lg, error) << "Point with ID=" << found_point->callID << " is now active (Sent SDP) and starting conf";
			mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseOK(200, ""));
			return; 
		}
		/*if (ActivePoints() >= 3) state = true;
		SendToCnfModulCR(); //true-false inside
		BOOST_LOG_SEV(lg, error) << "Point with ID=" << found_point->callID << " is now active (Sent SDP)";
		mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseOK(200, ""));
		return;*/
	}
	else if (action_result == "1") //client changed SDP
	{
		BOOST_LOG_SEV(lg, error) << "Point with ID=" << found_point->callID << " changed SDP:mode=" << found_point->state;
		mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseOK(200, "") + "\n\n" + found_point->serverSDP);
		return;
	}
	else // some error
	{
		BOOST_LOG_SEV(lg, fatal) << "Cnf::MDCX(...): default error:" << action_result;
		mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, action_result));
		return;
	}

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Cnf::DLCX(SHP_MGCP mgcp_)
{
	BOOST_LOG_SEV(lg, trace) << "Cnf::DLCX(...) for eventID=" << mgcp_->data[MGCP::EventID];
	SHP_CallerBase found_point = FindCallerBase(mgcp_);
	if (found_point == nullptr)
	{
		mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, "Error. Client could not be found."));
		return "";
	}
	string action_result = found_point->serverPort;
	BOOST_LOG_SEV(lg, error) << "DELETED point with ID=" << found_point->callID;
	RemoveCallerBase(found_point);
	if (ActivePoints() < 2) 
	{
		state = false;
		BOOST_LOG_SEV(lg, error) << "Set state=" << state;
	}

	SendToCnfModulMD_DL(found_point);
	mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseOK(250, ""));
	return action_result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Cnf::CheckExistance(SHP_MGCP mgcp_)
{
	string result;
	for (auto& point : vecCallerBase)
	{
		if (point->callID == mgcp_->data[MGCP::ParamC])
			return point->callID;
	}
	return "-1"; //return -1 - not found, -2 - error in data, else eventID
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_CallerBase Cnf::FindCallerBase(SHP_MGCP mgcp_)
{
	for (auto &point : vecCallerBase)
	if (point->callID == mgcp_->data[MGCP::ParamC]) return point;
	return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::RemoveCallerBase(SHP_CallerBase point_)
{
	vecCallerBase.erase(std::remove(vecCallerBase.begin(), vecCallerBase.end(), point_), vecCallerBase.end());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int Cnf::ActivePoints()
{
	int result = 0;
	for (auto &e: vecCallerBase) if (e->state) result++;
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::SendToCnfModulCR()
{
	if (state)
	{
		string clientIP = "";
		string clientPort = "";
		string serverPort = "";
		BOOST_LOG_SEV(lg, trace) << "Cnf::SendToCnfModulCR(): Chacking points:";
		for (auto &e : vecCallerBase)
		{
			BOOST_LOG_SEV(lg, debug) << "Point: ID=" << e->callID << " State=" << state;
			if (e->state)
			{
				BOOST_LOG_SEV(lg, debug) << "Adding to message: IP=" << e->clientIP << " Clientport=" << e->clientPort << " ServerPort=" << e->serverPort;
				clientIP += " " + e->clientIP;
				clientPort += " " + e->clientPort;
				serverPort += " " + e->serverPort;
			}
		}
		clientIP.erase(0, 1);//удаляем лишний пробел
		clientPort.erase(0, 1);
		serverPort.erase(0, 1);
		string result = "M7S2I6P5M\n";
		result += "From=mgcp\n";
		result += "To=cnf\n";
		result += "EventID=mgcp" + eventID + "\n";
		result += "EventType=cr\n";
		result += "ClientIP=" + clientIP + "\n";
		result += "ClientPort=" + clientPort + "\n";
		result += "ServerPort=" + serverPort + "\n";
		net_Data->SendModul(NETDATA::cnf, result);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::SendToCnfModulMD_DL(SHP_CallerBase point_)
{
	if (state)
	{
		string result = "M7S2I6P5M\n";
		result += "From=mgcp\n";
		result += "To=cnf\n";
		result += "EventID=mgcp" + eventID + "\n";
		result += "EventType=md\n";
		result += "ClientIP=" + point_->clientIP + "\n";
		result += "ClientPort=" + point_->clientPort + "\n";
		result += "ServerPort=" + point_->serverPort + "\n";
		net_Data->SendModul(NETDATA::cnf, result);
	}
	else
	{
		if (!deleted)
		{
			string result = "M7S2I6P5M\n";
			result += "From=mgcp\n";
			result += "To=cnf\n";
			result += "EventID=mgcp" + eventID + "\n";
			result += "EventType=dl\n";
			deleted = true;
			net_Data->SendModul(NETDATA::cnf, result);
		}
	}
	return;
}