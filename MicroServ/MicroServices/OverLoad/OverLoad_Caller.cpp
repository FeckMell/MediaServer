#include "stdafx.h"
#include "OverLoad_Caller.h"
using namespace OverLoad;

Caller::Caller(SHP_IPL ipl_)
{
	LOG::Log("debug", "OverLoad", "OverLoad::Caller::Caller _" + ipl_->Param("CallID") + "_");
	mapData.insert({ "CallID", ipl_->Param("CallID") });
	mapData.insert({ "ClientIP", ipl_->Param("ClientIP") });
	mapData.insert({ "ClientPort", ipl_->Param("ClientPort") });
	mapData.insert({ "ReplyTo", ipl_->Param("ReplyTo") });
}
string Caller::Param(string name_)
{
	return mapData[name_];
}