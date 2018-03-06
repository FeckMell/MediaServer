#include "stdafx.h"
#include "Parser.h"

MGCP::MGCP(string req)
{
	mgcp = req;
	error = 0;
	Parse();
}
//-------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------
std::string MGCP::ResponseBAD(int code, string message)
{
	auto response = boost::to_string(code) + " " + boost::to_string(stoi(MessNum) + 1) + " BAD";
	if (message != "")
	{
		response += "\nZ: " + message;
	}
	return response;
}
//-------------------------------------------------------------------------------------
void MGCP::Parse()
{
	Remove();
	parseCMD();
	EventP();
	ParamM();
	ParamL();
	ParamI();
	ParamZ();
	ParamC();
	ParamS();
	//CLogger.AddToLog(0, "\nCMD_" + CMD + "_M_" + paramM + "_L_" + paramL + "_C_" + paramC + "_I_" + paramI + "_Z_" + paramZ + "_S_" + paramS + "_Event_"+EventEx+"_");
	auto fd = mgcp.find("v=0");
	if (fd != std::string::npos){ SDP = mgcp.substr(fd); }
}
//-------------------------------------------------------------------------------------
void MGCP::parseCMD()
{
	std::string temp = mgcp.substr(0, 4);
	if (temp == "RQNT"){ CMD = "RQNT"; }
	else if (temp == "MDCX"){ CMD = "MDCX"; }
	else if (temp == "CRCX"){ CMD = "CRCX"; }
	else if (temp == "DLCX"){ CMD = "DLCX"; }
	else{ error = -1; }
} //DONE
//-------------------------------------------------------------------------------------
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
	if (mgcp.substr(5, 2) == "  "){ MessNum = mgcp.substr(7, fd - 8); }
	else{ MessNum = mgcp.substr(6, fd - 7); }
	//out << "\n----"+EventEx+"----\n";
}
//-------------------------------------------------------------------------------------
void MGCP::ParamM()
{
	if (mgcp.find("confrnce") != std::string::npos){ paramM = "confrnce"; }
	else if (mgcp.find("inactive") != std::string::npos){ paramM = "inactive"; }
	else if (mgcp.find("sendrecv") != std::string::npos){ paramM = "sendrecv"; }

}
//-------------------------------------------------------------------------------------
void MGCP::ParamL()
{
	auto fd = mgcp.find("L:");
	if (fd != std::string::npos)
	{
		paramL = mgcp.substr(fd + 3, mgcp.find("\n", fd) - fd - 3);
	}
	else { paramL = ""; }
}
//-------------------------------------------------------------------------------------
void MGCP::ParamI()
{
	auto fd = mgcp.find("I:");
	if (fd != std::string::npos)
	{
		paramI = mgcp.substr(fd + 3, mgcp.find("\n", fd) - fd - 3);
	}
	else { paramI = ""; }
}
//-------------------------------------------------------------------------------------
void MGCP::ParamZ()
{
	auto fd = mgcp.find("Z:");
	if (fd != std::string::npos)
	{
		paramZ = mgcp.substr(fd + 3, mgcp.find("\n", fd) - fd - 3);
	}
	else { paramZ = ""; }
}
//-------------------------------------------------------------------------------------
void MGCP::ParamC()
{
	auto fd = mgcp.find("C:");
	if (fd != std::string::npos)
	{
		paramC = mgcp.substr(fd + 3, mgcp.find("\n", fd) - fd - 3);
		return;
	}
	fd = mgcp.find("X:");
	if (fd != std::string::npos)
	{
		paramC = mgcp.substr(fd + 3, mgcp.find("\n", fd) - fd - 3);
		return;
	}
	else { paramC = ""; }
}
//-------------------------------------------------------------------------------------
void MGCP::ParamS()
{
	auto fd = mgcp.find("S:");
	if (fd != std::string::npos)
	{
		paramS = mgcp.substr(fd + 3, mgcp.find("\n", fd) - fd - 3);
	}
	else { paramS = ""; }
}
//-------------------------------------------------------------------------------------
void MGCP::Remove()
{
	auto fd = mgcp.find("\r");
	while (fd != std::string::npos)
	{
		mgcp.erase(mgcp.begin() + fd);
		fd = mgcp.find("\r");
	}
	/*fd = mgcp.find("\v");
	while (fd != std::string::npos)
	{
	mgcp.erase(mgcp.begin() + fd);
	fd = mgcp.find("\v");
	}
	fd = mgcp.find("\t");
	while (fd != std::string::npos)
	{
	mgcp.erase(mgcp.begin() + fd);
	fd = mgcp.find("\t");
	}
	fd = mgcp.find("\a");
	while (fd != std::string::npos)
	{
	mgcp.erase(mgcp.begin() + fd);
	fd = mgcp.find("\a");
	}*/
}
//-------------------------------------------------------------------------------------
bool MGCP::Valid()
{ 
	if (error == -1) return false;
	else return true;
}
//-------------------------------------------------------------------------------------