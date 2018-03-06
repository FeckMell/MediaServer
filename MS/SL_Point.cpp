#include "stdafx.h"
#include "SL_Point.h"


Point::Point(string callID_, SHP_SDP s_SDP_) :callID(callID_), serverSDP(s_SDP_)
{
	clientSDP.reset(new SDP());
	socket = SOCKSTORE::GetSocket(GetParam("ServerPort"));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_SDP Point::GetSDP(string which_)
{
	if (which_ == "server") return serverSDP;
	else if (which_ == "client") return clientSDP;
	else
	{
		cout << "\nSHP_SDP Point::GetSDP(string which_) wrong sdp type";
		return nullptr;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Point::GetParam(string param_)
{
	if (param_ == "CallID") return callID;
	else if (param_ == "State") return state;
	else if (param_ == "Error") return error;
	else if (param_ == "ServerPort") return serverSDP->data["Port"];
	else if (param_ == "ServerIP") return serverSDP->data["IP"];
	else if (param_ == "ClientPort") return clientSDP->data["Port"];
	else if (param_ == "ClientIP") return clientSDP->data["IP"];
	else
	{
		LOG::Log(LOG::fatal, "ERRORS", "Point class reports error in \"GetParam\":\nparam_ is unknown! Sent=" + param_ + "\n");
		exit(-1);
		return "";
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::ModifyClientSDP(SHP_SDP new_sdp_)
{
	if (clientSDP->sdp == "")
	{
		if (new_sdp_->sdp == "")
		{
			LOG::Log(LOG::fatal, "ERRORS", "Point class reports error in \"ModifyClientSDP\":\n NEW SDP IS EMPTY\n" + GetFullInfo());
			error = "bad sdp";
			return;
		}
		else
		{
			if (new_sdp_->data["Mode"] == "sendrecv")
			{
				state = "ready";
			}
			else if (new_sdp_->data["Mode"] == "inactive")
			{
				state = "paused";
			}
			clientSDP = new_sdp_;
			socket->SetEndPoint(GetParam("ClientIP"), GetParam("ClientPort"));
		}
	}
	else
	{
		if (new_sdp_->sdp == "")
		{
			LOG::Log(LOG::fatal, "ERRORS", "Point class reports error in \"ModifyClientSDP\":\n NEW SDP IS EMPTY\n" + GetFullInfo());
			error = "bad sdp";
			return;
		}
		else
		{
			if (clientSDP->data["Mode"] != new_sdp_->data["Mode"])
			{
				if (new_sdp_->data["Mode"] == "sendrecv")
				{
					state == "ready";
				}
				else if (new_sdp_->data["Mode"] == "inactive")
				{
					state = "paused";
				}
				clientSDP = new_sdp_;
				serverSDP->ChangeModeS(new_sdp_->data["Mode"]);
			}
			else
			{
				LOG::Log(LOG::fatal, "ERRORS", "Point class reports error in \"ModifyClientSDP\":\n NEW SDP STATE SAME AS OLD\n" + GetFullInfo() + "\n\nProblem SDP:\n" + new_sdp_->sdp);
				error = "bad sdp";
				return;
			}
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Point::GetFullInfo()
{
	string result = "";
	result += "\nPoint callID=" + callID + "_";
	result += "\nPoint state=" + state + "_";
	result += "\nPoint error=" + error + "_";

	result += "\nPoint My IP=" + socket->s.local_endpoint().address().to_string() + "_";
	result += "\nPoint My Port=" + to_string(socket->s.local_endpoint().port()) + "_";
	result += "\nPoint Client IP=" + socket->endPoint.address().to_string() + "_";
	result += "\nPoint Client Port=" + to_string(socket->endPoint.port()) + "_";

	result += "\nPoint time start=" + to_iso_extended_string(timer.GetStart()) + "_";
	result += "\nPoint time last=" + to_iso_extended_string(timer.GetLast()) + "_";
	result += "\nPoint time total=" + to_string(timer.GetFullDuration().seconds()) + " seconds_";

	result += "\nPoint serverSDP=\n" + serverSDP->sdp + "_";
	result += "\nPoint clientSDP=\n" + clientSDP->sdp + "_";

	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
map<string, SHP_Point> POINTSTORE::mapPoints = {};
//*///------------------------------------------------------------------------------------------
SHP_Point POINTSTORE::GetPoint(string callID_)
{
	SHP_Point found_point = mapPoints[callID_];
	if (found_point == nullptr)
	{
		mapPoints.erase(callID_);
		return nullptr;
	}
	else return found_point;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Point POINTSTORE::CreatePoint(string callID_, SHP_SDP s_SDP_)
{
	SHP_Point new_point; new_point.reset(new Point(callID_, s_SDP_));
	mapPoints[callID_] = new_point;
	return new_point;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void POINTSTORE::DeletePoint(string callID_)
{
	mapPoints.erase(callID_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------