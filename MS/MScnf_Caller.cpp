#include "stdafx.h"
#include "MScnf_Caller.h"
using namespace cnf;


Caller::Caller(string call_id_)
{
	basePoint = POINTSTORE::GetPoint(call_id_);

	InitCodec(&iccx, true);
	InitCodec(&occx, false);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Caller::~Caller()
{
	avcodec_close(iccx);
	avcodec_close(occx);
	avcodec_free_context(&iccx);
	avcodec_free_context(&occx);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::InitCodec(AVCodecContext** xccx_, bool mode_)//true-decoder, false-encoder
{

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

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::SetJitterSize(int size_)
{
	bufFrame.Resize(size_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Caller::StoreFrame(SHP_FRAME frame_, int which_)
{
	bufFrame.Push(frame_, which_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_FRAME Caller::GetFrame(int which_)
{
	return bufFrame.Pop(which_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------