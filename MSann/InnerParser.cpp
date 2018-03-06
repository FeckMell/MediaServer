#include "InnerParser.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
vector<string> IPL::paramNamesStr = 
	{ "modulName", "eventType", "clientIP", "clientPort", "serverPort", "eventID", "fileName", "maxParamNames" };
vector<string> IPL::eventTypeStr = { "cr", "md" "dl", "maxEventType" };
string IPL::myModulSTR = "";
int IPL::myModulINT = -100;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
IPL::IPL(char* rawMes_, boost::asio::ip::udp::endpoint sender_) : stringIPL(rawMes_), sender(sender_)
{
	data.resize(maxParamNames, "");
	Parse();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void IPL::Parse()
{
	for (int i = modulName; i < maxParamNames; ++i)
		data[i] = get_substr(stringIPL, EnumToStr(0, i) + "=", "\n");
	if (data[modulName] != /*myModulSTR*/"ann") error = "wrong modul";
	if ((type = StrToEnum(data[eventType])) == -1) error = "wrong event type";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string IPL::ResponseOK(int code_, string include_Event_Type_)
{
	return "NOT DONE";
}
string IPL::ResponseBAD(int code_, string message_)
{
	return "NOT DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string IPL::EnumToStr(int enum_, int num_)
{
	switch (enum_)
	{
	case 0:
	{
			  if (num_<0 || num_>maxParamNames) return "error num_";
			  else return paramNamesStr[num_];
	}
	case 1:
	{
			  if (num_<0 || num_>maxEventType) return "error num_";
			  else return eventTypeStr[num_];
	}
	default: return "error enum_";
	}
}
int IPL::StrToEnum(string name_)
{
	for (int i = 0; i < maxParamNames; ++i){ if (paramNamesStr[i] == name_) return i; }
	for (int i = 0; i < maxEventType; ++i){ if (eventTypeStr[i] == name_) return i; }
	return -1;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string IPL::PrintAll()
{
	string result = "";//"\nAll:\n1) Struct event: ";
	//result += "\nMessage to modul: " + EnumToStr(0, modulName);
	//result += "\nEvent: " + EnumToStr(1, eventType);
	result += "\nStruct data:\n";
	for (int i = 0; i < maxParamNames; ++i)
		result += "_" + EnumToStr(0, i) + "=_" + data[i] + "_;\n";
	result += "Error:_" + error + "_\n";
	result += "============================================================================";
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------