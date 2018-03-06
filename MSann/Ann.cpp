#include "stdafx.h"
#include "Ann.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Ann::Ann(SHP_MediaFile mediafile_, SHP_IPL ipl_)
{
	annID = ipl_->data[IPL::eventID];
	mediaFile = mediafile_;
	outerSocket.reset(new Socket(
		init_Params->data[IPar::outerIP], //my IP
		stoi(ipl_->data[IPL::serverPort]), // my port
		ioAnn));
	endPoint = udp::endpoint(
		boost::asio::ip::address::from_string(ipl_->data[IPL::clientIP]),
		stoi(ipl_->data[IPL::clientPort])
		);

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
		SHP_CAVPacket packet_to_send = CreatePacket(current_packet_num);
		SendPacket(packet_to_send);
		current_packet_num = (current_packet_num + 1) % file_size;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_CAVPacket Ann::CreatePacket(int current_packet_num_)
{
	SHP_CAVPacket file_packet = mediaFile->GetPacket(current_packet_num_);
	SHP_CAVPacket packet_to_send = make_shared<CAVPacket>(172);
	memcpy(packet_to_send->Data(), (uint8_t*)&rtpHDR.Get(), 12);
	memcpy(packet_to_send->Data() + 12, file_packet->Data(), 160);
	return packet_to_send;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::SendPacket(SHP_CAVPacket packet_to_send_)
{
	this_thread::sleep_for(std::chrono::milliseconds(20));
	outerSocket->s.send_to(boost::asio::buffer(packet_to_send_->Data(), packet_to_send_->Size()), endPoint);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::DL()
{
	state = false;
	th->join();
}