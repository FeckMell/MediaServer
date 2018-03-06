#include "stdafx.h"
#include "BasicStructs.h"



SOCK::SOCK(string ip_, int port_, IO& io_) :s(io_)
{
	using boost::asio::ip::udp;
	EP ep(boost::asio::ip::address::from_string(ip_), port_);
	s.open(udp::v4());
	s.set_option(udp::socket::reuse_address(true));
	s.bind(EP(boost::asio::ip::address::from_string(ip_), port_));

	BOOST_LOG_SEV(LOG::GL(LOG::L::main), trace) << "SOCK::SOCK(...) created with IP=" << ip_ << " port=" << port_;
}
SOCK::~SOCK()
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::main), trace) << "SOCK::SOCK(...) CLOSED with IP=" << s.local_endpoint().address().to_string() << " port=" << s.local_endpoint().port();
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