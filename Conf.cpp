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
void CConfRoom::NewInitPoint(string input_SDP, string output_SDP)
{
	loggit("void CConfRoom::NewInitPoint(string SDP)");
	SHP_CConfPoint point(new CConfPoint());
	point->SetSDP(input_SDP);
	string addr = Make_addr_from_SDP(output_SDP);
	point->Set_output_addr(addr);
	cllPoints_.push_back(point);
	loggit("void CConfRoom::NewInitPoint(string SDP) ENDED");
}
//--------------------------------------------------------------------------
void CConfRoom::Start()
{
	loggit("CConfRoom::Start()");
	cout << "\nCConfRoom::Start()";
	std::vector<string> SDPs;
	std::vector<string> addr;
	string logSDP = "";
	string logAddr = "";
	for (auto &entry : cllPoints_)
	{
		logAddr += entry->Get_output_addr() + "\n";
		logSDP += entry->GetSDP() + "\n\n";
		addr.push_back(entry->Get_output_addr());
		SDPs.push_back(entry->GetSDP());
	}
	loggit("Creating Mixer for SDPs:"+ logSDP + "\nand addresses:\n" + logAddr);
	SHP_CRTPReceive mix(new CRTPReceive(SDPs,addr));
	Mixer = mix;
	loggit("\nmix->process_all();");
	cout << "\nmix->process_all();";
	mix->process_all();
}
//--------------------------------------------------------------------------