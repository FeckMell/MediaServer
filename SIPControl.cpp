#include "stdafx.h"
#include "SIPControl.h"

void SIPControl::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	string time = "";
	steady_clock::time_point t1 = steady_clock::now();
	time += DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	time += " thread=" + boost::to_string(this_thread::get_id());
	CLogger->AddToLog(9, "\n" + time + "     " + a);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPControl::proceedINVITE(SIP &sip)
{
	server->reply(sip.ResponseRING(), sip.sender, CMGCPServer::sip);
	sip.SDP = "\nv=0\no=- 6666 230 IN IP4 10.77.7.20\ns=SIP Call\nc=IN IP4 10.77.7.20\nt=0 0\nm=audio 33000 RTP/AVP 8 101\na=sendrecv\na=rtpmap:8 PCMA/8000\na=ptime:20\na=rtpmap:101 telephone-event/8000\na=fmtp:101 0-11,16\n";
	server->reply(sip.ResponseOK(), sip.sender, CMGCPServer::sip);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPControl::proceedACK(SIP &sip)
{
	return;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPControl::proceedBYE(SIP &sip)
{
	return;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int SIPControl::GetFreePort()
{
	return server->GetFreePort();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SIPControl::SetFreePort(int port)
{
	server->SetFreePort(port);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------