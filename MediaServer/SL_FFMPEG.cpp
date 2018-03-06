#include "stdafx.h"
#include "SL_FFMPEG.h"

PACKET::PACKET()
{
	av_init_packet(&packet);
	packet.data = nullptr;
	packet.size = 0;
}
//*///------------------------------------------------------------------------------------------
PACKET::PACKET(size_t sz)
{
	if (sz > 0)
		av_new_packet(&packet, sz);
	else
	{
		av_init_packet(&packet);
		packet.data = nullptr;
		packet.size = 0;
	}
}
//*///------------------------------------------------------------------------------------------
PACKET::~PACKET()
{
	Free();
}
//*///------------------------------------------------------------------------------------------
AVPacket* PACKET::Get()
{
	return &packet;
}
//*///------------------------------------------------------------------------------------------
int PACKET::Size()
{
	return packet.size;
}
//*///------------------------------------------------------------------------------------------
uint8_t* PACKET::Data()
{
	return packet.data;
}
//*///------------------------------------------------------------------------------------------
void PACKET::Free()
{
	av_free_packet(&packet);
}
//*///------------------------------------------------------------------------------------------
void PACKET::MakeSize(int n)
{
	packet.size = n;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
FRAME::FRAME()
{
	frame = av_frame_alloc();
	empty = false;
}
//*///------------------------------------------------------------------------------------------
FRAME::FRAME(bool a)
{
	frame = av_frame_alloc();
	empty = true;
}
//*///------------------------------------------------------------------------------------------
FRAME::~FRAME()
{
	av_frame_free(&frame);
}
//*///------------------------------------------------------------------------------------------
AVFrame* FRAME::Get()
{
	return frame;
}
//*///------------------------------------------------------------------------------------------
void FRAME::Free()
{
	av_frame_free(&frame);
}
//*///------------------------------------------------------------------------------------------
bool FRAME::Empty()
{
	return empty;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_PACKET FFF::AddToPacket(SHP_PACKET packet1_, SHP_PACKET packet2_)
{
	if (packet1_ == nullptr)
	{
		return CreatePacket(packet2_->Data(), packet2_->Size());
	}
	else
	{
		SHP_PACKET result = make_shared<PACKET>(packet1_->Size() + packet2_->Size());
		memcpy(result->Data(), packet1_->Data(), packet1_->Size());
		memcpy(result->Data()+packet1_->Size(), packet2_->Data(), packet2_->Size());
		return result;
	}
}
//*///------------------------------------------------------------------------------------------
SHP_PACKET FFF::CreatePacket(uint8_t* data_, int size_)
{
	SHP_PACKET result = make_shared<PACKET>(size_);
	memcpy(result->Data(), data_, size_);
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
subjitter::subjitter() : frames(6)
{

}
//*///------------------------------------------------------------------------------------------
void subjitter::Push(SHP_FRAME frame_)
{
	frames.push_back(frame_);
}
//*///------------------------------------------------------------------------------------------
SHP_FRAME subjitter::Pop()
{
	if (frames.empty() == true) return nullptr;

	SHP_FRAME result = frames.front();
	frames.pop_front();
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
JITTER::JITTER()
{
	buffers.resize(3);
}
//*///------------------------------------------------------------------------------------------
void JITTER::Resize(int size_)
{
	buffers.clear();
	buffers.resize(size_);
}
//*///------------------------------------------------------------------------------------------
void JITTER::Push(SHP_FRAME frame_, int which_)
{
	buffers[which_].Push(frame_);
}
//*///------------------------------------------------------------------------------------------
SHP_FRAME JITTER::Pop(int which_)
{
	return buffers[which_].Pop();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------