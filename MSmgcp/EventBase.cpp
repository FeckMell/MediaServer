#include "EventBase.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
vector<int> EventBase::usedPorts;
vector<int> EventBase::usedEventNum;
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
	int free_port = stoi(init_Params->data[IPar::rtpPort]);
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
string EventBase::ReserveEventNum()
{
	int free_event_num = 0;
	if (usedEventNum.size() == 0) { usedEventNum.push_back(free_event_num);  return to_string(free_event_num); }
	for (unsigned i = 0; i < usedEventNum.size(); ++i)
	{
		if (usedEventNum[i] != free_event_num)
		{
			usedEventNum.push_back(free_event_num);
			sort(usedEventNum.begin(), usedEventNum.end());
			return to_string(free_event_num);
		}
		free_event_num++;
	}
	usedEventNum.push_back(free_event_num);
	sort(usedEventNum.begin(), usedEventNum.end());
	return to_string(free_event_num);
}
void EventBase::FreeEventNum(string num_)
{
	usedEventNum.erase(remove(usedEventNum.begin(), usedEventNum.end(), stoi(num_)), usedEventNum.end());
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
		"m=audio %2% RTP/AVP 8\n"//2
		"a=rtpmap:8 PCMA/8000\n"
		"a=ptime:20\n"
		"a=sendrecv\n"
		)); // формируем тип ответа
	auto result = str(template_sdp
		%init_Params->data[IPar::outerIP]
		% port_s_
		%lastSDP_ID
		%mgcp_->data[MGCP::ParamC]
		);
	lastSDP_ID++;
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------