#pragma once
#include "stdafx.h"
namespace mgcp
{
	class MGCP
	{
	public:
		MGCP(char*, EP);
		void ReplyClient();
		void ReplyNOTMGCP();

		string PrintAll();

		EP sender;
		string request;
		string mgcp = "";
		string sdp = "";
		string serverSDP = "";

		string outerError = "";
		string innerError = "";
		map<string, string> data;

	private:

		string ResponseOK();
		string ResponseBAD();

		void Remove();
		void SplitMGCPandSDP();
		void ParseMain();
		void ParseRest();

		void CheckValid();
	};

	typedef shared_ptr<MGCP> SHP_MGCP;
}
