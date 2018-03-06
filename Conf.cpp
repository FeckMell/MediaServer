#include "stdafx.h"
#include "Conf.h"
void CConfPoint::loggit(string a)
{
	fprintf(FileLogConfPoint, ("\n" + a + "\n//-------------------------------------------------------------------").c_str());
	fflush(FileLogConfPoint);
}
void CConfRoom::loggit(string a)
{
	fprintf(FileLogConfRoom, ("\n" + a + "\n//-------------------------------------------------------------------").c_str());
	fflush(FileLogConfRoom);
}
//--------------------------------------------------------------------------
string CConfRoom::Make_addr_from_SDP(string output_SDP)
{
	string result = "rtp://";
	string temp;
	std::size_t found = output_SDP.find("c=IN IP4");
	if (found != std::string::npos)
		temp = output_SDP.substr(found + 9, output_SDP.find("\n", found+1)-found - 9);
	result += temp + ":";
	found = output_SDP.find("m=audio");
	if (found != std::string::npos)
		temp = output_SDP.substr(found + 8, output_SDP.find(" ", found + 10)- found - 8);
	result += temp;
	return result;
}
//--------------------------------------------------------------------------
void CConfRoom::NewInitPoint(string SDP, string CallID, int port)
{
	loggit("void CConfRoom::NewInitPoint(string SDP)");
	SHP_CConfPoint point(new CConfPoint());

	point->SetID(CallID);
	point->SetMyPort(port);
	point->SetSDP(SDP);
	point->ModifySDP();
	
	point->SetRemotePort(atoi(MakeRemotePort(SDP).c_str()));
	point->SetRemoteIP(MakeRemoteIP(SDP));

	cllPoints_.push_back(point);
	loggit("void CConfRoom::NewInitPoint(string SDP) ENDED");
}
//--------------------------------------------------------------------------
void CConfRoom::Start()
{
	loggit("CConfRoom::Start()");
	std::vector<string> SDPs;
	std::vector<string> IPs;
	std::vector<int> ports1;
	std::vector<int> ports2;
	string logSDP = "\n";
	for (auto &entry : cllPoints_)
	{
		logSDP += entry->GetSDP() + "\n";

		SDPs.push_back(entry->GetSDP());
		IPs.push_back(entry->GetRemoteIP());
		ports1.push_back(entry->GetMyPort());
		ports2.push_back(entry->GetRemotePort());

	}
	loggit("Creating Mixer for SDPs:"+ logSDP);
	Mixer.reset(new CRTPReceive(SDPs, IPs, ports1, ports2));
	loggit("mix->process_all();");
	Mixer->process_all();
}
//--------------------------------------------------------------------------
SHP_CConfPoint CConfRoom::FindPoint(string CallID)
{
	for (auto &e : cllPoints_)
	{
		if (e->GetID() == CallID)
			return e;
	}
	assert(true);
	return NULL;
}
//--------------------------------------------------------------------------
string CConfRoom::MakeRemoteIP(string SDP)
{
	std::size_t found = SDP.find("c=IN IP4");
	if (found != std::string::npos)
		return SDP.substr(found + 9, SDP.find("\n", found + 1) - found - 9);
	return "";
}
//--------------------------------------------------------------------------
string CConfRoom::MakeRemotePort(string SDP)
{
	std::size_t found = SDP.find("m=audio");
	if (found != std::string::npos)
		return SDP.substr(found + 8, SDP.find(" ", found + 10) - found - 8);
	return "";
}
//--------------------------------------------------------------------------
void CConfPoint::ModifySDP()
{
	std::size_t found = SDP_.find("m=audio");
	if (found != std::string::npos)
		SDP_ = SDP_.replace(found + 8, SDP_.find(" ", found + 10) - found - 8, to_string(GetMyPort() - 2000));
}