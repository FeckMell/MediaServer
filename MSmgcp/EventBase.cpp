#include "stdafx.h"
#include "EventBase.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
vector<int> EventBase::usedPorts;
vector<int> EventBase::usedEventID;
int EventBase::lastSDP_ID = 0;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
EventBase::EventBase()
{
	BOOST_LOG_SEV(lg, trace) << "EventBase::EventBase()";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string EventBase::ReservePort()
{
	int free_port = stoi(init_Params->data[STARTUP::rtpPort]);
	if (usedPorts.size() == 0)
	{
		usedPorts.push_back(free_port);
		return to_string(free_port);
	}
	for (unsigned i = 0; i < usedPorts.size(); ++i)
	{
		if (usedPorts[i] != free_port)
		{
			usedPorts.push_back(free_port);
			sort(usedPorts.begin(), usedPorts.end());
			return to_string(free_port);
		}
		free_port += 2;
	}
	usedPorts.push_back(free_port);
	sort(usedPorts.begin(), usedPorts.end());
	return to_string(free_port);
}
void EventBase::FreePort(string port_)
{
	usedPorts.erase(remove(usedPorts.begin(), usedPorts.end(), stoi(port_)), usedPorts.end());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string EventBase::ReserveEventID()
{
	int free_event_id = 0;
	if (usedEventID.size() == 0) { usedEventID.push_back(free_event_id);  return to_string(free_event_id); }
	for (unsigned i = 0; i < usedEventID.size(); ++i)
	{
		if (usedEventID[i] != free_event_id)
		{
			usedEventID.push_back(free_event_id);
			sort(usedEventID.begin(), usedEventID.end());
			return to_string(free_event_id);
		}
		free_event_id++;
	}
	usedEventID.push_back(free_event_id);
	sort(usedEventID.begin(), usedEventID.end());
	return to_string(free_event_id);
}
void EventBase::FreeEventID(string event_id_)
{
	usedEventID.erase(remove(usedEventID.begin(), usedEventID.end(), stoi(event_id_)), usedEventID.end());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string EventBase::GenSDP(string port_s_, SHP_MGCP mgcp_)
{
	auto template_sdp = boost::format(string(
		"v=0\n"
		"o=- %3% 0 IN IP4 %1%\n"//3,1
		"s=%4%\n"//4
		"c=IN IP4 %1%\n"//1
		"t=0 0\n"
		"a=tool:libavformat 57.3.100\n"
		"m=audio %2% RTP/AVP 8 101\n"//2
		"a=rtpmap:8 PCMA/8000\n"
		"a=rtpmap:101 telephone-event/8000\n"
		"a=ptime:20\n"
		"a=sendrecv\n"
		)); // формируем тип ответа
	auto result = str(template_sdp
		%init_Params->data[STARTUP::outerIP]
		% port_s_
		%lastSDP_ID
		%mgcp_->data[MGCP::ParamC]
		);
	lastSDP_ID++;
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------