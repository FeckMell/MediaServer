#include "CnfPoint.h"

CnfPoint::CnfPoint(string c_port_, string s_port_, string c_IP_, string s_IP_, boost::asio::io_service& ioCnf_)
	: ioCnf(ioCnf_), clientPort(c_port_), serverPort(s_port_), clientIP(c_IP_), serverIP(s_IP_)
{
	BOOST_LOG_SEV(lg, debug) << "CnfPoint::CnfPoint(...) with serverport=" << s_port_;
	socket.reset(new SOCK(
		serverIP, //my IP
		stoi(serverPort), // my port
		ioCnf));
	endPoint = EP(
		boost::asio::ip::address::from_string(clientIP),
		stoi(clientPort)
		);
	BOOST_LOG_SEV(lg, trace) << "CnfPoint::CnfPoint(...): net init DONE";
	InitCodec(&iccx, true);
	BOOST_LOG_SEV(lg, trace) << "CnfPoint::CnfPoint(...): InitCodec(&iccx, true); DONE";
	InitCodec(&occx, false);
	BOOST_LOG_SEV(lg, trace) << "CnfPoint::CnfPoint(...): InitCodec(&occx, false); DONE";
}
CnfPoint::~CnfPoint()
{
	BOOST_LOG_SEV(lg, debug) << "CnfPoint::~CnfPoint() with serverport=" << serverPort;
	avcodec_close(iccx);
	avcodec_close(occx);
	avcodec_free_context(&iccx);
	avcodec_free_context(&occx);
	BOOST_LOG_SEV(lg, debug) << "CnfPoint::~CnfPoint() DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void CnfPoint::InitCodec(AVCodecContext** xccx_, bool mode_)//true-decoder, false-encoder
{
	BOOST_LOG_SEV(lg, trace) << "CnfPoint::InitCodec()";
	AVCodec *x_codec;//possible leak
	if (mode_) x_codec = avcodec_find_decoder(AV_CODEC_ID_PCM_ALAW);
	else x_codec = avcodec_find_encoder(AV_CODEC_ID_PCM_ALAW);

	(*xccx_) = (avcodec_alloc_context3(x_codec));

	(*xccx_)->channels = 1;
	(*xccx_)->channel_layout = av_get_default_channel_layout((*xccx_)->channels);
	(*xccx_)->sample_fmt = x_codec->sample_fmts[0];
	(*xccx_)->sample_rate = 8000;
	(*xccx_)->bit_rate = 8000;
	(*xccx_)->time_base = { 1, (*xccx_)->sample_rate };

	avcodec_open2((*xccx_), x_codec, nullptr);
	BOOST_LOG_SEV(lg, trace) << "CnfPoint::InitCodec() DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void CnfPoint::SetMaxTimesTook(int t_)
{
	timesTookMax = t_ - 1;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_FRAME CnfPoint::GetFrame()
{
	if (timesTook >= timesTookMax) return nullptr;
	timesTook++;
	return bufFrame;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void CnfPoint::StoreFrame(SHP_FRAME f_)
{
	bufFrame = f_;
	timesTook = 0;
}