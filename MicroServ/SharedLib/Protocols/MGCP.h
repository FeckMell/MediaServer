#pragma once
#include "../../stdafx.h"
#include "../Basic/Simple.h"
#include "../Basic/Functions.h"
#include "SDP.h"

class MGCP
{
public:

	MGCP(SHP_SockBuf request_, EP ep_);

	/*ReplyFuncs*/
	string Reply();
	string ReplyNTFY();
	/*#ReplyFuncs*/

	/*Data Access*/
	SHP_SDP GetSDP(string type_);
	void SetSDP(string type_, SHP_SDP new_sdp_);

	string Param(string name_);
	void SetParam(string name_, string value_);

	string PrintAll();
	/*#Data Access*/

private:

	MGCP();
	/*Parcing funcs*/
	void RemoveExtra();
	void SplitMGCPandSDP();
	void ParseMain();
	void ParseRest();
	void CheckValid();
	/*#Parcing funcs*/

	/*ReplyFuncs*/
	string ResponseOK();
	/*#ReplyFuncs*/

	/*Data*/
	EP endPoint;
	map<string, string> mapData;
	map<string, SHP_SDP> mapSDP;// = { { "Client", nullptr }, { "Server", nullptr } };
	/*#Data*/

}; typedef shared_ptr<MGCP> SHP_MGCP;

