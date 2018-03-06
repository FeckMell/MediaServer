#include "stdafx.h"
#include "Cnf.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Cnf::Cnf(SHP_MGCP mgcp_)
{
	eventNum = mgcp_->data[MGCP::EventNum];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::CRCX(SHP_MGCP mgcp_, string server_sdp_, string server_port_)
{
	SHP_CallerBase new_point = make_shared<CallerBase>(CallerBase(mgcp_, server_sdp_, server_port_));
	vecCallerBase.push_back(new_point);
	ReplyClient(mgcp_, mgcp_->ResponseOK(200, "add event type") + "\n\n" + server_sdp_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::MDCX(SHP_MGCP mgcp_)
{
	SHP_CallerBase found_point = FindCallerBase(mgcp_);
	if (found_point==nullptr)
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found.3"));
		return;
	}
	string action_result = found_point->ModifyCallerBase(mgcp_);
	if (action_result == "0") // client sent first SDP
	{
		if (ActivePoints() >= 3) state = true;
		SendToCnfModulCR(); //true-false inside
		ReplyClient(mgcp_, mgcp_->ResponseOK(200, ""));
		return;
	}
	else if (action_result == "1") //client changed SDP
	{

		ReplyClient(mgcp_, mgcp_->ResponseOK(200, "") + "\n\n" + found_point->serverSDP);
		return;
	}
	else // some error
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, action_result));
		return;
	}

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Cnf::DLCX(SHP_MGCP mgcp_)
{
	SHP_CallerBase found_point = FindCallerBase(mgcp_);
	if (found_point == nullptr)
	{
		ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Error. Client could not be found."));
		return "";
	}
	string action_result = found_point->serverPort;
	RemoveCallerBase(found_point);
	if (ActivePoints() < 2) state = false;

	SendToCnfModulMD_DL(found_point);
	ReplyClient(mgcp_, mgcp_->ResponseOK(250, ""));
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
	return "-1"; //return -1 - not found, -2 - error in data, else eventNum
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
		for (auto &e : vecCallerBase)
		{
			if (e->state)
			{
				clientIP += " " + e->clientIP;
				clientPort += " " + e->clientPort;
				serverPort += " " + e->serverPort;
			}
		}
		cout << "\nDEBUG:";
		cout << "\nclientIP=" << clientIP << "\nclientPort=" << clientPort << "\nserverPort=" << serverPort;
		clientIP.erase(0, 1);//удаляем лишний пробел
		clientPort.erase(0, 1);
		serverPort.erase(0, 1);
		string result = "modulName=cnf\n";
		result += "eventType=cr\n";
		result += "clientIP=" + clientIP + "\n";
		result += "clientPort=" + clientPort + "\n";
		result += "serverPort=" + serverPort + "\n";
		result += "eventID=" + eventNum + "\n";
		SendModul(NETDATA::cnf, result);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::SendToCnfModulMD_DL(SHP_CallerBase point_)
{
	if (state)
	{
		string result = "modulName=cnf\n";
		result += "eventType=md\n";
		result += "clientIP=" + point_->clientIP + "\n";
		result += "clientPort=" + point_->clientPort + "\n";
		result += "serverPort=" + point_->serverPort + "\n";
		result += "eventID=" + eventNum + "\n";
		SendModul(NETDATA::cnf, result);
	}
	else
	{
		if (!deleted)
		{
			string result = "modulName=cnf\n";
			result += "eventType=dl\n";
			result += "eventID=" + eventNum + "\n";
			deleted = true;
			SendModul(NETDATA::cnf, result);
		}
	}
	return;
}