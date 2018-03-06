#include "stdafx.h"
#include "FFmpeg.h"

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
	av_free_packet(&packet);
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
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------