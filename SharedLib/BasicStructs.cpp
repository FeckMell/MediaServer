#include "stdafx.h"
#include "BasicStructs.h"


SOCK::SOCK(string ip_, int port_, SHP_IO io_) :s(*io_.get()), io(io_)
{
	EP ep(boost::asio::ip::address::from_string(ip_), port_);
	s.open(boost::asio::ip::udp::v4());
	s.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	s.bind(ep);
}
void SOCK::ChangeIO(SHP_IO io_)
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