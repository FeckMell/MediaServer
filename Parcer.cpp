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
	ParamC();
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