#include "stdafx.h"
#include "MSann_Ann.h"
using namespace ann;

Ann::Ann(SHP_MediaFile mediafile_, SHP_IPL ipl_)
{
	
	eventID = ipl_->data["EventID"];
	callID = ipl_->data["CallID"];
	loop = ipl_->data["Loop"];

	mediaFile = mediafile_;
	
	outerSOCK = SSTORAGE::GetSocket(ipl_->data["ServerPort"]);
	endPoint = EP(
		boost::asio::ip::address::from_string(ipl_->data["ClientIP"]),
		stoi(ipl_->data["ClientPort"])
		);
	LOG::Log(LOG::info, "ANN", "MSANN: ann with id="+eventID+" started");
	th.reset(new std::thread(&Ann::Run, this));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::Run()
{
	int file_size = mediaFile->Size();
	int current_packet_num = 0;
	int times = 0;
	
	while (state == true)
	{
		SHP_PACKET packet_to_send = CreatePacket(current_packet_num);
		SendPacket(packet_to_send);
		current_packet_num = (current_packet_num + 1) % file_size;
		if (current_packet_num == 0)
		{
			if (loop == "once")
			{
				state = false;
				SendModul("Loop=end\n");
			}
			times++;
			if (times == 2)
			{
				break;
			}
		}
		/*if (loop == "once" && current_packet_num == 0)
		{
			state = false;
			SendModul("Loop=end\n");
		}*/
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_PACKET Ann::CreatePacket(int current_packet_num_)
{
	SHP_PACKET file_packet = mediaFile->GetPacket(current_packet_num_);
	SHP_PACKET packet_to_send = make_shared<PACKET>(172);
	memcpy(packet_to_send->Data(), (uint8_t*)&(outerSOCK->rtp.Get()), 12);
	memcpy(packet_to_send->Data() + 12, file_packet->Data(), 160);
	return packet_to_send;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::SendPacket(SHP_PACKET packet_to_send_)
{
	this_thread::sleep_for(std::chrono::milliseconds(20));
	outerSOCK->s.send_to(boost::asio::buffer(packet_to_send_->Data(), packet_to_send_->Size()), endPoint);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::DL()
{
	LOG::Log(LOG::info, "ANN", "MSANN: Ann::DL ann with id="+eventID+" finished");
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
	result += "CallID=" + callID + "\n";
	
	if (modul == "mgcp") NET::outerIO->post(boost::bind(NET::SignalInIO, NET::INNER::mgcp, result));
	else if (modul == "sip") NET::outerIO->post(boost::bind(NET::SignalInIO, NET::INNER::sip, result));
	else LOG::Log(LOG::fatal, "ANN", "MSANN: ann with id=" + eventID + " sendmodul ERROR");

}