#pragma once
#include "../SharedSource/stdafx.h"
#include "../SharedSource/Structs.h"
#include "../SharedSource/Functions.h"

class IPL
{
public:
	/*Enum struct data*/
	enum ParamNames
	{//             int type
		modulName, eventType, clientIP, clientPort, serverPort, eventID,
		maxParamNames
	};
	enum EventType { cr, md, dl, maxEventType };

	/*Main public activity*/
	IPL(char*, EP);
	string ResponseOK(int, string);
	string ResponseBAD(int, string);

	/*sub functions to make string names from enumed params and back*/
	string EnumToStr(int, int);
	int StrToEnum(string);

	/*Access to parsed data*/
	string PrintAll();

	/*Data storage*/
	int type = -1;
	vector<string> data;
	string stringIPL;
	EP sender;
	string error = "";

private:
	/*Parsing functions*/
	void Parse();

	/*String names for enumed data*/
	static vector<string> paramNamesStr;//0
	static vector<string> eventTypeStr;//1
};
typedef shared_ptr<IPL> SHP_IPL;