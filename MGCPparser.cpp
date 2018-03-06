#include "stdafx.h"
#include "MGCPparser.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
vector<string> MGCP::paramNamesStr = { "C", "L", "I", "Z", "S", "EventNum", "EventFull", "MessNum", "SDP", "MaxParamNames" };
vector<string> MGCP::eventNamesStr = { "CMD", "Type", "State", "MaxEventName" };
vector<string> MGCP::eventCMDStr = { "CRCX", "RQNT", "MDCX", "DLCX", "MaxEventCMD" };
vector<string> MGCP::eventTypeStr = { "ann", "cnf", "prx", "MaxEventType" };
vector<string> MGCP::eventStatesStr = { "confrnce", "inactive", "sendrecv", "MaxEventState" };
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
MGCP::MGCP(char* rawMes_, boost::asio::ip::udp::endpoint sender_) : MGCPstring(rawMes_), sender(sender_)
{
	data.resize(MaxParamNames, "");
	events.resize(MaxEventName, -1);
	Parse();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::Parse()
{
	Remove();
	ParseCMDandType();
	ParseRest();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::Remove()
{
	auto fd_pos = MGCPstring.find("\r");
	while (fd_pos != string::npos)
	{
		MGCPstring.erase(MGCPstring.begin() + fd_pos);
		//cout << "\nDEBUG \"r\":" << fd_pos;
		fd_pos = MGCPstring.find("\r", fd_pos - 1);
	}
	auto fd_pos2 = MGCPstring.find("  ");
	while (fd_pos2 != string::npos)
	{
		MGCPstring.erase(MGCPstring.begin() + fd_pos2);
		fd_pos2 = MGCPstring.find("  ", fd_pos2 - 1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::ParseCMDandType()
{
	/*temp data*/
	string temp = MGCPstring.substr(0, 4);
	size_t fd_pos;

	/*parse event state*/
	for (int i = 0; i < MaxEventState; ++i) if (fd_pos = MGCPstring.find(EnumToStr(4,i)) != string::npos) { events[State] = i; break; }
	if ((MGCPstring.find("\nM:") != string::npos) && (events[State] == -1)) 
	{ error += "\nUnknown event state. Supported: confrnce, inactive, sendrecv."; return; }
	
	/*parse CMD*/
	for (int i = 0; i < MaxEventCMD; ++i) if (temp == EnumToStr(2,i)){ events[CMD] = i; break; } 
	if (events[CMD] == -1) { error += "\nUnknown command. Supported: CRCX, MDCX, DLCX, RQNT."; return; }

	/*parse event type*/
	temp = MGCPstring.substr(0, MGCPstring.find("\n"));
	for (int i = 0; i < MaxEventType; ++i) if ((fd_pos = temp.find(EnumToStr(3, i))) != string::npos){ events[Type] = i; break; }
	if (events[Type] == -1) { error += "\nUnknown type. Supported: ann/, cnf/, prx/."; return; }

	/*parse extra event data*/
	if (temp.find("@") == string::npos) { error += "\nError in first line. It should be:\n\"CMD MesNum EventType/EventNum@[IP] mgcp 1.0 ncs 1.0\"."; return; }
	try
	{
		string temp2 = temp.substr(fd_pos + 4, temp.find("@") - fd_pos - 4);
		if (temp2 == "$") { data[EventNum] = "$"; }
		else { stoi(temp2); data[EventNum] = temp2; }
	}
	catch (exception& e) { error += "\nInvalid event type num."; e; return; }
	data[EventFull] = temp.substr(0, temp.find("] ", fd_pos) + 1);
	data[MessNum] = get_substr(temp, EnumToStr(2, events[CMD])+" ", " "+EnumToStr(3, events[Type]));
	try{ stoi(data[MessNum]); }
	catch (exception& e) { error += "\nInvalid message num."; e; return; }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCP::ParseRest()
{
	for (int i = ParamC; i <= ParamS; ++i) data[i] = get_substr(MGCPstring, "\n" + EnumToStr(0, i) + ": ", "\n");
	if (data[ParamC] == "") { data[ParamC] = get_substr(MGCPstring, "\nX: ", "\n"); }
	auto fd_pos = MGCPstring.find("v=0");
	if (fd_pos != string::npos){ data[SDP] = MGCPstring.substr(fd_pos); }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MGCP::ResponseOK(int code_, string include_Event_Type_)
{
	auto result = to_string(code_) + " " + data[MessNum]+ " OK";
	if (include_Event_Type_ != "")
	{
		result += "\nZ: " + EnumToStr(3, events[Type]) + "/" + data[EventNum] + cut_substr(data[EventFull], "@[", "]");
		result += "\nI: " + to_string(rand() % 1000);
	}
	return result;
}
string MGCP::ResponseBAD(int code_, string message_)
{
	auto result = boost::to_string(code_) + " " + boost::to_string(stoi(data[MessNum]) + 1) + " BAD";
	if (message_ != ""){ result += "\nZ: " + message_; }
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MGCP::EnumToStr(int enum_, int num_)
{
	switch (enum_)
	{
		case 0:
		{
			  if (num_<0 || num_>MaxParamNames) return "error num_";
			  else return paramNamesStr[num_];
		}
		case 1:
		{
			  if (num_<0 || num_>MaxEventName) return "error num_";
			  else return eventNamesStr[num_];
		}
		case 2:
		{
			  if (num_<0 || num_>MaxEventCMD) return "error num_";
			  else return eventCMDStr[num_];
		}
		case 3:
		{
			  if (num_<0 || num_>MaxEventType) return "error num_";
			  else return eventTypeStr[num_];
		}
		case 4:
		{
			  if (num_<0 || num_>MaxEventState) return "error num_";
			  else return eventStatesStr[num_];
		}
		default: return "error enum_";
	}
}
int MGCP::StrToEnum(string name_)
{
	for (int i = 0; i < MaxParamNames; ++i){ if (paramNamesStr[i] == name_) return i; }
	for (int i = 0; i < MaxEventName; ++i){ if (eventNamesStr[i] == name_) return i; }
	for (int i = 0; i < MaxEventType; ++i){ if (eventTypeStr[i] == name_) return i; }
	for (int i = 0; i < MaxEventCMD; ++i){ if (eventCMDStr[i] == name_) return i; }
	for (int i = 0; i < MaxEventState; ++i){ if (eventStatesStr[i] == name_) return i; }
	return -1;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MGCP::PrintAll()
{
	string result = "\nAll:\n1) Struct event: ";
	for (int i = 0; i < MaxEventName; ++i)
		result += "_" + EnumToStr(1, i) + "_=_" + to_string(events[i]) + "_=_" + EnumToStr(i + 2, events[i]) + "_; ";
	result += "\n2) Struct data:\n";
	for (int i = 0; i < MaxParamNames;++i)
		result += "_" + EnumToStr(0, i) + "=_" + data[i] + "_;\n";
	result += "Error:_" + error + "_\n";
	result += "============================================================================";
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------