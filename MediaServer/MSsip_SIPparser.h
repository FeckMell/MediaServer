#pragma once
#include "stdafx.h"
#include "SharedLib.h"


namespace sip
{
	class SIP
	{
	public:

		SIP(char*, EP);

		void ReplyClient();

		SHP_SDP clientSDP;
		SHP_SDP serverSDP;
		string outerError = "";
		string innerError = "";

		map<string, string> data;

	private:

		void Remove(string&);
		void SplitSIPandSDP(string&);
		void ParseMain();
		void Check();

		string ReplyRinging();
		string ReplyOK();
		void SendClient(string);

		EP sender;
		string sip = "";
		
	};
	typedef shared_ptr<SIP> SHP_SIP;
}