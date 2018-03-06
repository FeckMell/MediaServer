#include "stdafx.h"
#include "SL_BasicStructs.h"


SOCK::SOCK(string ip_, int port_, SHP_IO io_) :s(*io_.get()), io(io_)
{
	EP ep(boost::asio::ip::address::from_string(ip_), port_);
	s.open(boost::asio::ip::udp::v4());
	s.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	s.bind(ep);
}
void SOCK::ChangeIO(SHP_IO io_)
{
	if (io_ != io)
	{
		auto ep_old = s.local_endpoint();
		s.cancel();
		s.close();

		s = boost::asio::ip::udp::socket(*io_.get());
		s.open(boost::asio::ip::udp::v4());
		s.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		s.bind(ep_old);
		io = io_;
	}
}
void SOCK::AsyncReceive(boost::function<void(boost::system::error_code ec_, size_t size_)> boostbind_)
{
	s.async_receive_from(
		boost::asio::buffer(buffer, 2048), 
		endPoint, 
		boostbind_);
}
void SOCK::SetEndPoint(string ip_, string port_)
{
	endPoint = EP(boost::asio::ip::address::from_string(ip_), stoi(port_));
}
void SOCK::SendTo(uint8_t* what_, int size_)
{
	s.send_to(boost::asio::buffer(what_, size_), endPoint);
}
uint8_t* SOCK::GetRTP()
{
	return (uint8_t*)&rtp.Get();
}
SOCK::~SOCK()
{
	s.cancel();
	s.close();
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
RTP_struct::RTP_struct()
{
	Config();
}
//*///------------------------------------------------------------------------------------------
void RTP_struct::Config()
{
	this->header.version = 2;
	this->header.marker = 0;
	this->header.csrc_len = 0;
	this->header.extension = 0;
	this->header.padding = 0;
	this->header.ssrc = htons(10);
	this->header.payload_type = 8;
	this->header.timestamp = htonl(0);
	this->header.seq_no = htons(0);
}
//*///------------------------------------------------------------------------------------------
RTP RTP_struct::Get()
{
	++this->amount;
	this->header.seq_no = htons(this->amount);
	this->header.timestamp = htonl(160 * this->amount);

	return header;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------