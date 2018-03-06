#include "stdafx.h"
#include "SrcAsio.h"
//#define _DEBUG

/************************************************************************
	CSrcAsio
************************************************************************/
CSrcAsio::CSrcAsio(const string& sdp_file,
	boost::asio::io_service& io_service, unsigned short port)
	: CSrcCommon(sdp_file)
{
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

	pSocket_.reset(new udp::socket(io_service, udp::endpoint(udp::v4(), port)));

	auto decoder = avcodec_find_decoder(idCodec);
	RETURN_ON_AVERROR(
		avcodec_open2(codecRTP_, decoder, NULL),
		boost::format("Failed to open decoder for file %1%") % sdp_file);

	_do_receive();
}

//-----------------------------------------------------------------------
SHP_CScopedPFrame CSrcAsio::getNextDecoded(bool& bEOF)
{
	SHP_CScopedPFrame frameNULL;
	SHP_CScopedPFrame frame = std::make_shared<CScopedPFrame>();
	bEOF = !frame->isValid();
	if (bEOF)	//С памятью проблемы
		return frameNULL;


	SHP_CAVPacket encPack = popEncoded();
	if (!encPack)
		return frameNULL;


	int got_frame = 0;
	m_lastError = avcodec_decode_audio4(codecRTP_, *frame, &got_frame, encPack.get());

	if (!got_frame || m_lastError < 0)
		return frameNULL;

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

	auto packEnc = circular_.pop(); //blocked reading
/*
	cout << boost::format("popEncoded in thread %1%\n") %
		std::this_thread::get_id();*/
	return packEnc;
}

//-----------------------------------------------------------------------
void CSrcAsio::_do_receive()
{
	enum { RTP_HEADER_SIZE = 12 };
	printf("RTP _do_recieve\n");
	pSocket_->async_receive_from(
		boost::asio::buffer(data_, max_length), endpSender_,
		[this](boost::system::error_code ec, std::size_t bytes_recvd)
	{
		if (bytes_recvd > RTP_HEADER_SIZE)
		{
			const size_t szPack = bytes_recvd - RTP_HEADER_SIZE;
			SHP_CAVPacket shpPacket = std::make_shared<CAVPacket>(szPack);
			memcpy(shpPacket->data, data_ + RTP_HEADER_SIZE, szPack);
			/*
			shpPacket->pts = shpPacket->dts = pts_;
			pts_ += szPack;*/
			circular_.push(shpPacket);
//putbacktocomments
///*
#ifdef _DEBUG
			cout << boost::format("%1% recieved %2% bytes in thread %3%\n") 
				% Name()
				% bytes_recvd
				% std::this_thread::get_id()
				;
#endif // _DEBUG
//*/

			//cout << boost::format("Recieved\t%1%\t%2%\n") % endpSender_ % bytes_recvd;
		}
		_do_receive();
	});
}
