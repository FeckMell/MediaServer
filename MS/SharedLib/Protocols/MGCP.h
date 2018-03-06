#pragma once
#include "../../stdafx.h"
#include "SDP.h"

class MGCP
{
public:

	MGCP(string request_, EP endpoint_);
	MGCP(){}

	void ReplyClient();
	void ReplyNTFY();

	string ReplyClient2();
	string ReplyNTFY2();

	SHP_SDP GetSDP(string type_);
	void SetSDP(string type_, SHP_SDP new_sdp_);
	void SetParam(string name_, string value_);
	string Param(string name_);
	string PrintAll();

	
	SHP_SDP clientSDP;
	SHP_SDP serverSDP;

private:
	void SendClient(string);
	string ResponseOK();

	void Remove();
	void SplitMGCPandSDP();
	void ParseMain();
	void ParseRest();

	void CheckValid();

	EP endPoint;
	map<string, string> mapData;

};

typedef shared_ptr<MGCP> SHP_MGCP;

