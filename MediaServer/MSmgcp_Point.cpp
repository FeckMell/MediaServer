#include "stdafx.h"
#include "MSmgcp_Point.h"
using namespace mgcp;


Point::Point(SHP_MGCP mgcp_)
{
	callID = mgcp_->data["CallID"];
	serverSDP = mgcp_->serverSDP;

	if (mgcp_->clientSDP == nullptr){ state = false; }
	else
	{
		clientSDP = mgcp_->clientSDP;
		if (clientSDP->data["Mode"] == "sendrecv") state = true;
		else state = false;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::ModifyPoint(SHP_MGCP mgcp_)
{
	if (clientSDP != nullptr && mgcp_->clientSDP != nullptr)
	{
		if (clientSDP->data["Mode"] != mgcp_->clientSDP->data["Mode"])
		{
			clientSDP = mgcp_->clientSDP;
			serverSDP->ChangeModeS(mgcp_->clientSDP->data["Mode"]);
			mgcp_->serverSDP = serverSDP;
			if (clientSDP->data["Mode"] == "sendrecv") state = true;
			else state = false;
		}
	}
	else if (clientSDP == nullptr && mgcp_->clientSDP != nullptr)
	{
		clientSDP = mgcp_->clientSDP;
		if (mgcp_->clientSDP->data["Mode"] == "sendrecv") state = true;
		else state = false;
	}
	else
	{
		mgcp_->innerError = "Modify what ERROR";
	}
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
/*string Point::PrintPoint()
{
	string result = "\n==========================================================================================================\n";
	result += "==========================================================================================================\n";
	result += "\nCallID=" + callID + "\n";
	result += "EventID=" + eventID + "\n";
	//result += "\nclientSDP=" + clientSDP->sdp + "\n";
	//result += "\nserverSDP=" + serverSDP + "\n";
	result += "\nclientPort=" + clientPort + "\n";
	result += "\nserverPort=" + serverPort + "\n";
	result += "\nclientIP=" + clientIP + "\n";
	result += "==========================================================================================================\n\n\n";
	return result;
}*/