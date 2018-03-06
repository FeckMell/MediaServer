#include "stdafx.h"
#include "MSann_Ann.h"
using namespace ann;

Ann::Ann(SHP_MediaFile media_file_, SHP_IPL ipl_)
{
	mediaFile = media_file_;
	eventID = ipl_->data["EventID"];
	loop = ipl_->data["Loop"];
	basePoint = POINTSTORE::GetPoint(ipl_->data["CallID"]);

	th.reset(new std::thread(&Ann::Run, this));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Ann::GetParam(string param_)
{
	if (param_ == "Loop") return loop;
	else if (param_ == "EventID") return eventID;
	else return basePoint->GetParam(param_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::Run()
{
	int file_size = mediaFile->Size();
	int current_packet_num = 0;

	while (state == true)
	{
		SHP_PACKET packet_to_send = CreatePacket(current_packet_num);
		SendPacket(packet_to_send);
		current_packet_num = (current_packet_num + 1) % file_size;
		if (loop == "once" && current_packet_num == 0)
		{
			state = false;
			SendModul("LoopEnd");
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_PACKET Ann::CreatePacket(int current_packet_num_)
{
	SHP_PACKET rtp_packet = make_shared<PACKET>(basePoint->socket->GetRTP(), 12);
	rtp_packet->Add(mediaFile->GetPacket(current_packet_num_)->Get());
	return rtp_packet;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::SendPacket(SHP_PACKET packet_to_send_)
{
	this_thread::sleep_for(std::chrono::milliseconds(20));
	basePoint->socket->SendTo(packet_to_send_->Data(), packet_to_send_->Size());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::DL()
{
	state = false;
	th->join();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::SendModul(string event_)
{
	string modul;
	if (eventID.find("mgcp") != string::npos) modul = "mgcp";
	if (eventID.find("sip") != string::npos) modul = "sip";

	string result = "";
	result += "From=ann\n";
	result += "To=" + modul + "\n";
	result += "EventID=" + eventID + "\n";
	result += "EventType=" + event_ + "\n";
	result += "CallID=" + GetParam("CallID") + "\n";

	if (modul == "mgcp") COM::outerIO->post(boost::bind(COM::SignalInIO, COM::INNER::mgcp, result));
	else if (modul == "sip") COM::outerIO->post(boost::bind(COM::SignalInIO, COM::INNER::sip, result));
	else LOG::Log(LOG::fatal, "ERRORS", "MSANN: ann with id=" + eventID + " sendmodul ERROR");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------