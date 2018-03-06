#include "stdafx.h"
#include "Ann.h"
using namespace ann;

Ann::Ann(SHP_MediaFile mediafile_, SHP_IPL ipl_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Ann::Ann(...) for ann " << ipl_->data["EventID"];
	annID = ipl_->data["EventID"];
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Ann::Ann(...): annID = ipl_->data[IPL::eventID];->mediaFile = mediafile_;";
	mediaFile = mediafile_;
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Ann::Ann(...): mediaFile = mediafile_;->init socket and endpoint";
	outerSOCK = SSTORAGE::GetSocket(ipl_->data["ServerPort"]);
	endPoint = EP(
		boost::asio::ip::address::from_string(ipl_->data["ClientIP"]),
		stoi(ipl_->data["ClientPort"])
		);
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Ann::Ann(...):init socket and endpoint DONE->th.reset(new std::thread(&Ann::Run, this));";
	th.reset(new std::thread(&Ann::Run, this));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::Run()
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Ann::Run() "<<annID;
	int file_size = mediaFile->Size();
	int current_packet_num = 0;
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Ann::Run(): init DONE, start send in while";
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
	memcpy(packet_to_send->Data(), (uint8_t*)&rtpHDR.Get(), 12);
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
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Ann::DL() " << annID;
	state = false;
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), debug) << "Ann::DL() th->join(); ?";
	th->join();
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), debug) << "Ann::DL() thread joined";
}