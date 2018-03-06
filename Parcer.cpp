#include "stdafx.h"
#include "Parser.h"
#include "Functions.h"

MGCP::MGCP(string req) :mgcp(req)
{
	error = 0;
	Parse(false);
}
//*///------------------------------------------------------------------------------------------
string MGCP::ResponseOK(int code, string end)
{
	auto response = boost::to_string(code) + " " + /*boost::to_string(stoi(MessNum) + 1)*/MessNum + " OK";
	if (end != "")
	{
		response += "\nZ: " + end + EventNum + EventEx.substr(EventEx.find("@"));
		response += "\nI: " + boost::to_string(rand() % 1000);
	}
	return response;
}
//*///------------------------------------------------------------------------------------------
std::string MGCP::ResponseBAD(int code, string message)
{
	auto response = boost::to_string(code) + " " + boost::to_string(stoi(MessNum) + 1) + " BAD";
	if (message != "")
	{
		response += "\nZ: " + message;
	}
	return response;
}
//*///------------------------------------------------------------------------------------------
void MGCP::Parse(bool m)
{
	if(m) mgcp = string(mes);
	Remove();
	parseCMD();
	EventP();
	ParamM();
	//ParamL();
	//ParamI();
	//ParamZ();
	ParamC();
	//ParamS();
	paramL = get_substr(mgcp, "L: ", "\n");
	paramI = get_substr(mgcp, "I: ", "\n");
	paramZ = get_substr(mgcp, "Z: ", "\n");
	paramS = get_substr(mgcp, "S: ", "\n");
	//CLogger->AddToLog(0, "\nCMD_" + CMD + "_M_" + paramM + "_L_" + paramL + "_C_" + paramC + "_I_" + paramI + "_Z_" + paramZ + "_S_" + paramS + "_Event_"+EventEx+"_");
	auto fd = mgcp.find("v=0");
	if (fd != std::string::npos){ SDP = mgcp.substr(fd); }
}
//*///------------------------------------------------------------------------------------------
void MGCP::parseCMD()
{
	std::string temp = mgcp.substr(0, 4);
	if (temp == "RQNT"){ CMD = "RQNT"; }
	else if (temp == "MDCX"){ CMD = "MDCX"; }
	else if (temp == "CRCX"){ CMD = "CRCX"; }
	else if (temp == "DLCX"){ CMD = "DLCX"; }
	else{ error = -1; }
} //DONE
//*///------------------------------------------------------------------------------------------
void MGCP::EventP()
{
	auto fd = mgcp.find("ann/");
	if (fd != std::string::npos)
	{
		Event = ann;
		EventS = "ann/";
	}
	else if (mgcp.find("cnf/") != std::string::npos)
	{
		Event = cnf;
		EventS = "cnf/";
		fd = mgcp.find("cnf/");
	}
	else if (mgcp.find("prx/") != std::string::npos)
	{
		Event = prx;
		EventS = "prx/";
		fd = mgcp.find("prx/");
	}
	else { error = -1; return; }

	if (mgcp.find("@") == std::string::npos) { error = -1; return; }
	EventNum = mgcp.substr(fd + 4, mgcp.find("@") - fd - 4);
	EventEx = mgcp.substr(fd, mgcp.find("] ", fd) - fd + 1);
	MessNum = mgcp.substr(mgcp.find(" ") + 1, fd - 6);
}
//*///------------------------------------------------------------------------------------------
void MGCP::ParamM()
{
	if (mgcp.find("confrnce") != std::string::npos){ paramM = "confrnce"; }
	else if (mgcp.find("inactive") != std::string::npos){ paramM = "inactive"; }
	else if (mgcp.find("sendrecv") != std::string::npos){ paramM = "sendrecv"; }

}
//*///------------------------------------------------------------------------------------------
void MGCP::ParamC()
{
	paramC = get_substr(mgcp, "C: ", "\n");
	if (paramC == "") { paramC = get_substr(mgcp, "X: ", "\n"); }
}
//*///------------------------------------------------------------------------------------------
void MGCP::Remove()
{
	auto fd = mgcp.find("\r");
	while (fd != std::string::npos)
	{
		mgcp.erase(mgcp.begin() + fd);
		fd = mgcp.find("\r", fd - 1);
	}
	fd = mgcp.find("  ");
	while (fd != std::string::npos)
	{
		mgcp.erase(mgcp.begin() + fd);
		fd = mgcp.find("  ", fd - 1);
	}
}
//*///------------------------------------------------------------------------------------------
bool MGCP::Valid()
{ 
	if (error == -1) return false;
	else return true;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SIP::SIP(string)
{ 
	SIPLines.resize((int)line::lineMax);
}
//*///------------------------------------------------------------------------------------------
std::string SIP::ResponseBAD(int code, string message){ return " "; }
//*///------------------------------------------------------------------------------------------
void SIP::Parse()
{
	sip = string(mes);
	Remove();
	ParseToLines();
}
//*///------------------------------------------------------------------------------------------
void SIP::Remove()
{
	auto fd = sip.find("\r");
	while (fd != std::string::npos)
	{
		sip.erase(sip.begin() + fd);
		fd = sip.find("\r", fd - 1);
	}
	fd = sip.find("  ");
	while (fd != std::string::npos)
	{
		sip.erase(sip.begin() + fd);
		fd = sip.find("  ", fd - 1);
	}
}
//*///------------------------------------------------------------------------------------------
void SIP::ParseCMD()
{
	if (SIPLines[FirstLine].find("INVITE") != std::string::npos) CMD = INVITE;
	else if (SIPLines[FirstLine].find("ACK") != std::string::npos) CMD = ACK;
	else if (SIPLines[FirstLine].find("BYE") != std::string::npos) CMD = BYE;
	else if (SIPLines[FirstLine].find("OK") != std::string::npos) CMD = OK;
}
//*///------------------------------------------------------------------------------------------
void SIP::ParseToLines()
{
	SIPLines[FirstLine] = sip.substr(0, sip.find("\n"));
	SIPLines[Via] = get_substr(sip, "Via: ", "\n");
	SIPLines[To] = get_substr(sip, "To: ", "\n");
	SIPLines[From] = get_substr(sip, "From: ", "\n");
	SIPLines[CallID] = get_substr(sip, "Call-ID: ", "\n");
	SIPLines[CSeq] = get_substr(sip, "CSeq: ", "\n");
	SIPLines[Contact] = get_substr(sip, "Contact: ", "\n");
	SIPLines[ContentLen] = get_substr(sip, "Content-Length: ", "\n");
	SIPLines[CintentType] = get_substr(sip, "Content-Type: ", "\n");
	SIPLines[MaxForwards] = get_substr(sip, "Max-Forwards: ", "\n");
	SIPLines[Supported] = get_substr(sip, "Supported: ", "\n");
	SIPLines[Allow] = get_substr(sip, "Allow: ", "\n");
	SDP = sip.substr(sip.find("v=0"));
}
//*///------------------------------------------------------------------------------------------
void SIP::print()
{
	AddTagTo();
	cout << "\nPrinting:";
	for (int i = 0; i < (int)line::lineMax; ++i)
		cout << "\n+++" << SIPLines[i] << "+++\n";
	cout << "print finished!";
}
//*///------------------------------------------------------------------------------------------
void SIP::AddTagTo()
{
	auto a = std::chrono::steady_clock::now();
	SIPLines[To] += ";tag=" + std::to_string(a.time_since_epoch().count() % 10000000000);
}
//*///------------------------------------------------------------------------------------------
std::string SIP::ResponseOK()
{
	std::string result = "SIP/2.0 200 OK\n";
	result += "Via: " + SIPLines[Via] + ViaBranch + ViaReceived + "\n";
	result += "To: " + SIPLines[To] + "\n";
	result += "From: " + SIPLines[From] + "\n";
	result += "Call-ID: " + SIPLines[CallID] + "\n";
	result += "CSeq: " + SIPLines[CSeq] + "\n";
	result += "Contact: " + MyContact + "\n";
	result += "Content-Type: application/sdp\n";
	result += "Content-Length: " + std::to_string(SDP.length()) + "\n";
	result += "\n" + SDP;
	return result;
}
//*///------------------------------------------------------------------------------------------
std::string SIP::ResponseRING()
{
	AddTagTo();
	std::string result = "SIP/2.0 180 Ringing\n";
	result += "Via: " + SIPLines[Via] + ViaReceived + "\n";
	result += "To: " + SIPLines[To] + "\n";
	result += "From: " + SIPLines[From] + "\n";
	result += "Call-ID: " + SIPLines[CallID] + "\n";
	result += "CSeq: " + SIPLines[CSeq] + "\n";
	result += "Contact: " + MyContact + "\n";
	result += "Content-Length: 0";

	return result;
}
//*///------------------------------------------------------------------------------------------
std::string SIP::ResponseTRY(int code, std::string mess) { return"a"; }
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------