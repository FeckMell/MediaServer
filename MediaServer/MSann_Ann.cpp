#include "stdafx.h"
#include "MSann_Ann.h"
using namespace ann;

Ann::Ann(SHP_MediaFile mediafile_, SHP_IPL ipl_)
{
	
	eventID = ipl_->data["EventID"];
	
	mediaFile = mediafile_;
	
	outerSOCK = SSTORAGE::GetSocket(ipl_->data["ServerPort"]);
	endPoint = EP(
		boost::asio::ip::address::from_string(ipl_->data["ClientIP"]),
		stoi(ipl_->data["ClientPort"])
		);
	//BOOST_LOG_SEV(LOG::GL(0), info) << "MSANN: ann with id=" << eventID << " started";
	th.reset(new std::thread(&Ann::Run, this));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::Run()
{
	int file_size = mediaFile->Size();
	int current_packet_num = 0;
	
	while (state)
	{
		SHP_PACKET packet_to_send = CreatePacket(current_packet_num);
		SendPacket(packet_to_send);
		current_packet_num = (current_packet_num + 1) % file_size;
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
	BOOST_LOG_SEV(LOG::GL(0), info) << "MSANN: Ann::DL ann with id=" << eventID << " finished";
	state = false;
	th->join();
}