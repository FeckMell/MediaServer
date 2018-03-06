#include "stdafx.h"
#include "LogicAnnPoint.h"

LogicAnnPoint::LogicAnnPoint(SHP_SDP c_SDP_, SHP_SDP s_SDP_)
{
	_basecaller = CALLERSTORE::CreateCaller(s_SDP_);
	_basecaller->ModifyClientSDP(c_SDP_);
	_data["EventID"] = "mgcp_" + to_iso_extended_string(boost::posix_time::microsec_clock::local_time());
	_data["Error"] = _basecaller->Param("Error");
}
LogicAnnPoint::~LogicAnnPoint()
{
	if (_data["State"] == "PlayAnn")
	{
		NotifyModul("StopAnn");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicAnnPoint::RequestMusic(string filename_, string loop_)
{
	_data["Filename"] = get_substr(filename_, ",file:///", ")");
	_data["Loop"] = loop_;
	CheckFileExistance();
	NotifyModul("PlayAnn");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicAnnPoint::StopMusic()
{
	NotifyModul("StopAnn");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string LogicAnnPoint::Param(string name_)
{
	if (name_ == "CallID") return _basecaller->Param(name_);
	else if (name_ == "ClientIP") return _basecaller->Param(name_);
	else if (name_ == "ClientPort") return _basecaller->Param(name_);
	else if (name_ == "ServerIP") return _basecaller->Param(name_);
	else if (name_ == "ServerPort") return _basecaller->Param(name_);
	else if (name_ == "Error") return _data[name_] + _basecaller->Param(name_);
	else
	{
		return _data[name_];
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string LogicAnnPoint::PrintAll()
{
	string result = "Data in LogicAnnPoint object:\n";
	for (auto& e : _data)
	{
		result += e.first + "=" + e.second + "_\n";
	}
	result += "\n" + _basecaller->PrintAll();
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_CALLER LogicAnnPoint::GetCaller()
{
	return _basecaller;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicAnnPoint::CheckFileExistance()
{
	ifstream file(CFG::Param("MediaPath") + CFG::Param("Slash") + Param("FileName"));
	if (file.is_open()) 
	{ 
		file.close(); 
	}
	else
	{
		_data["Error"] += "File does not exists";
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogicAnnPoint::NotifyModul(string event_)
{
	if (_data["State"] == event_) 
	{ 
		_data["Error"] += "AlreadyPlayingAnn";
		return;
	}
	if (Param("Error") == "")
	{
		_data["State"] = event_;

		string result = "";
		result += "From=mgcp\n";
		result += "To=ann\n";
		result += "EventType=" + event_ + "\n";
		result += "EventID=" + Param("EventID") + "\n";
		result += "CallID=" + Param("CallID") + "\n";
		result += "FileName=" + Param("FileName") + "\n";
		result += "Loop=" + Param("Loop") + "\n";
		COM::SendToModul("ANN", result);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------