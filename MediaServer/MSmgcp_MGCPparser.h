#pragma once
#include "stdafx.h"
#include "SharedLib.h"


namespace mgcp
{
	class MGCP
	{
	public:

		MGCP(char*, EP);

		void ReplyClient();
		void ReplyNTFY();
		string PrintAll();

		EP sender;
		string request;
		string mgcp = "";
		SHP_SDP clientSDP;
		SHP_SDP serverSDP;

		string outerError = "";
		string innerError = "";

		map<string, string> data;

	private:
		void SendClient(string);
		string ResponseOK();

		void Remove();
		void SplitMGCPandSDP();
		void ParseMain();
		void ParseRest();

		void CheckValid();

	};

	typedef shared_ptr<MGCP> SHP_MGCP;
}
