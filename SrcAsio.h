#pragma once

#include "SrcCommon.h"
#include "Utils.h"
#include <boost/scoped_ptr.hpp>



/************************************************************************
	CSrcAsio
************************************************************************/
class CSrcAsio : public CSrcCommon
{
public:
	CSrcAsio(const string& sdp_string,
		boost::asio::io_service& io_service, unsigned short port, string IP);

	~CSrcAsio()	{
		avcodec_free_context(&codecRTP_);
	}
	const AVCodecContext *CodecCTX()const override{ return codecRTP_; };
	SHP_CScopedPFrame getNextDecoded(bool& bEOF) override;

private:
	void _do_receive();//2
	SHP_CAVPacket popEncoded();

	boost::scoped_ptr<udp::socket> pSocket_;
	AVCodecContext * codecRTP_ = nullptr;

	udp::endpoint endpSender_;
	int64_t pts_ = 0;
	int64_t frpts_ = 0;

	enum { max_length = 2048 };
	char data_[max_length];
	//boost::circular_buffer<SHP_CAVPacket> circular_;
	CThreadedCircular<SHP_CAVPacket> circular_{ 50, false };
	unsigned int port;
	string IP_;
	asio::io_service &io_service__;
};