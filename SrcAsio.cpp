#pragma once
#include "stdafx.h"
#include "SrcAsio.h"
#ifdef FFFFf
//#define _DEBUG

/************************************************************************
	CSrcAsio
************************************************************************/
CSrcAsio::CSrcAsio(const string& sdp_string,
	boost::asio::io_service& io_service, unsigned short port, string IP)
	: CSrcCommon(sdp_string), io_service__(io_service)
{
	cout << "\nASIO CONSTRUCT";
	CSrcAsio::port = port;
	IP_ = IP;
	cout << "\n-----------------------------------------------------------------------";
	cout << "\nport=" << port << " IP=" << IP_;
	cout << "\n-----------------------------------------------------------------------";
	if (m_lastError < 0)
		return;
	const auto idCodec = _CodecCTX()->codec_id;
	codecRTP_ = avcodec_alloc_context3(_CodecCTX()->codec);
	if (codecRTP_ == nullptr)
		RETURN_ON_AVERROR(AVERROR(ENOMEM), "avcodec_alloc_context3");
	RETURN_ON_AVERROR(
		avcodec_copy_context(codecRTP_, _CodecCTX()),
		"avcodec_copy_context"
		);
	_closeFormat();
	////
	//pSocket_.reset(new boost::asio::ip::udp::socket(io_service__));
	//pSocket_->open(boost::asio::ip::udp::v4());
	//pSocket_->set_option(boost::asio::socket_base::reuse_address(true));
	cout << "\nASIO CONSTRUCT 2";
	/////
	pSocket_.reset(new udp::socket(io_service, udp::endpoint(boost::asio::ip::address::from_string(/*IP_*/"10.77.7.19"), port)));
	
	cout << "\nSOKET=" << pSocket_->is_open();
	auto decoder = avcodec_find_decoder(idCodec);
	RETURN_ON_AVERROR(
		avcodec_open2(codecRTP_, decoder, NULL),
		boost::format("Failed to open decoder for string %1%") % sdp_string);
	
	cout << "\n CSrcAsio \n  do recieve";
	_do_receive();
}

//-----------------------------------------------------------------------
SHP_CScopedPFrame CSrcAsio::getNextDecoded(bool& bEOF)
{
	//cou << "\nASIO 1";
	SHP_CScopedPFrame frameNULL;
	//cou << "\nASIO 2";
	SHP_CScopedPFrame frame = std::make_shared<CScopedPFrame>();
	//cou << "\nASIO 3";
	bEOF = !frame->isValid();
	//cou << "\nASIO 4";
	if (bEOF)	//С памятью проблемы
		return frameNULL;

	//cout << "\n      1 ASIO 5 before popEncoded";
	SHP_CAVPacket encPack = popEncoded();// тут
	//cout << "\n      1 ASIO 6 after popEncoded";

	if (!encPack)
		return frameNULL;

	//cou << "\nASIO 7";
	int got_frame = 0;
	//cou << "\nASIO 8";
	m_lastError = avcodec_decode_audio4(codecRTP_, *frame, &got_frame, encPack.get());
	//cou << "\nASIO 9";
	if (!got_frame || m_lastError < 0)
		return frameNULL;
	cout << "\n      1 exit from getnext decoded(Asio)";
	return frame;
}

//-----------------------------------------------------------------------
SHP_CAVPacket CSrcAsio::popEncoded()
{
	/*
	if (circular_.empty())
		return SHP_CAVPacket();

	auto packEnc = circular_.front();
	circular_.pop_front();
	*/
	
	cout << "\n       1 Call from popEncoded (Asio) 1";
	auto packEnc = circular_.pop(); //blocked reading
	cout << "\n       1 exit from popEncoded (Asio) 2";
	
/*
	cou << boost::format("popEncoded in thread %1%\n") %
		std::this_thread::get_id();*/

	return packEnc;
}

//-----------------------------------------------------------------------
void CSrcAsio::_do_receive()
{
	enum { RTP_HEADER_SIZE = 12 };
	cout << "\n   in do_recieve";
	pSocket_->async_receive_from(
		boost::asio::buffer(data_, max_length), udp::endpoint(boost::asio::ip::address::from_string(/*IP_*/"10.77.7.19"), port),
		[this](boost::system::error_code ec, std::size_t bytes_recvd)
	{
		//cout << "   lymbdarecvd:" << bytes_recvd;
		if (bytes_recvd > RTP_HEADER_SIZE)
		{
			const size_t szPack = bytes_recvd - RTP_HEADER_SIZE;
			SHP_CAVPacket shpPacket = std::make_shared<CAVPacket>(szPack);
			memcpy(shpPacket->data, data_ + RTP_HEADER_SIZE, szPack);
			/*
			shpPacket->pts = shpPacket->dts = pts_;
			pts_ += szPack;*/
			cout << "\n    circular_.push(shpPacket);";
			circular_.push(shpPacket);
// вернуть комментарий ниже
/*
#ifdef _DEBUG
			cou << boost::format("%1% recieved %2% bytes in thread %3%\n") 
				% Name()
				% bytes_recvd
				% std::this_thread::get_id()
				;
#endif // _DEBUG
*/
		}
		//cout << "   call do_recieve recurcive";
		_do_receive();
	});
}
#endif