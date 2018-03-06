#include "stdafx.h"
#include "FCnf_Point.h"
using namespace NFCnf;

Point::Point(string call_id_)
{
	baseCaller = CallerStore::GetCaller(call_id_);
	InitCodec(&iccx, true);
	InitCodec(&occx, false);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Point::~Point()
{
	avcodec_close(iccx);
	avcodec_close(occx);
	avcodec_free_context(&iccx);
	avcodec_free_context(&occx);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::SetJitterSize(int size_)
{
	thisJitter.Resize(size_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::StoreFrame(SHP_FRAME frame_, int which_)
{
	thisJitter.Push(frame_, which_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_FRAME Point::GetFrame(int which_)
{
	return thisJitter.Pop(which_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::InitCodec(AVCodecContext** xccx_, bool type_)
{
	AVCodec *x_codec;//possible leak
	if (type_) x_codec = avcodec_find_decoder(AV_CODEC_ID_PCM_ALAW);
	else x_codec = avcodec_find_encoder(AV_CODEC_ID_PCM_ALAW);

	(*xccx_) = (avcodec_alloc_context3(x_codec));

	(*xccx_)->channels = 1;
	(*xccx_)->channel_layout = av_get_default_channel_layout((*xccx_)->channels);
	(*xccx_)->sample_fmt = x_codec->sample_fmts[0];
	(*xccx_)->sample_rate = 8000;
	(*xccx_)->bit_rate = 8000;
	(*xccx_)->time_base = { 1, (*xccx_)->sample_rate };

	avcodec_open2((*xccx_), x_codec, nullptr);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------