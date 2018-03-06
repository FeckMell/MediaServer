#pragma once
#include "stdafx.h"
namespace sip
{
	class SIP
	{
	public:
		enum { CMD, CallID, maxqqq };
		SIP(char*, EP);

		string GetParam(int);
		void ReplyClient(SHP_SOCK, string);

		string ReplyRinging();
		string ReplyOK(string);
		string ResponseBAD();

		EP sender;
		string request;
		string sip = "";
		string sdp = "";
		string error = "";
	private:
		void Remove();
		void SplitSIPandSDP();
		void ParseMain();

		string GetCMD();
		string GetCallID();

		vector<string> data;
	};
	typedef shared_ptr<SIP> SHP_SIP;
}