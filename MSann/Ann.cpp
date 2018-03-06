#include "Ann.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Ann::Ann(SHP_MediaFile mediafile_, SHP_IPL ipl_)
{
	BOOST_LOG_SEV(lg, trace) << "Ann::Ann(...) for ann " << ipl_->data[IPL::eventID];
	annID = ipl_->data[IPL::eventID];
	BOOST_LOG_SEV(lg, trace) << "Ann::Ann(...): annID = ipl_->data[IPL::eventID];->mediaFile = mediafile_;";
	mediaFile = mediafile_;
	BOOST_LOG_SEV(lg, trace) << "Ann::Ann(...): mediaFile = mediafile_;->init socket and endpoint";
	outerSocket.reset(new Socket(
		init_Params->data[IPar::outerIP], //my IP
		stoi(ipl_->data[IPL::serverPort]), // my port
		ioAnn));
	endPoint = udp::endpoint(
		boost::asio::ip::address::from_string(ipl_->data[IPL::clientIP]),
		stoi(ipl_->data[IPL::clientPort])
		);
	BOOST_LOG_SEV(lg, trace) << "Ann::Ann(...):init socket and endpoint DOBE->th.reset(new std::thread(&Ann::Run, this));";
	th.reset(new std::thread(&Ann::Run, this));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::Run()
{
	BOOST_LOG_SEV(lg, trace) << "Ann::Run() "<<annID;
	int file_size = mediaFile->Size();
	int current_packet_num = 0;
	BOOST_LOG_SEV(lg, trace) << "Ann::Run(): init DONE, start send in while";
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
	BOOST_LOG_SEV(lg, trace) << "Ann::DL() " << annID;
	state = false;
	BOOST_LOG_SEV(lg, debug) << "Ann::DL() th->join(); ?";
	th->join();
	BOOST_LOG_SEV(lg, debug) << "Ann::DL() thread joined";
}