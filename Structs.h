#pragma once
#include "stdafx.h"

struct SSource
{
	std::vector<AVFilterContext *> src;
};
struct Initing
{
	std::vector<AVFormatContext *> out_ifcx;
	std::vector<AVCodecContext *> out_iccx;

	std::vector<AVFormatContext *> ifcx;
	std::vector<AVCodecContext *> iccx;

	std::vector<SSource> afcx;

	std::vector<AVFilterGraph *> graphVec;
	std::vector<AVFilterContext *> sinkVec;
};
struct SdpOpaque
{
	using Vector = std::vector<uint8_t>; Vector data; Vector::iterator pos;
};
struct RTP
{
	uint8_t Csic : 4;
	uint8_t Extention : 1;
	uint8_t Padding : 1;
	uint8_t Ver : 2;
	uint8_t PT : 7;
	uint8_t Marker : 1;
	uint16_t SN : 16;
	uint8_t ESN : 4;
	uint32_t TS : 32;
	uint8_t SSI : 7;
	uint8_t *data;
	void setdata(uint8_t * a){ data = a; SN++; ESN++; TS += 160; }
};
struct CAVPacket2 : AVPacket
{
	CAVPacket2() : AVPacket()
	{
		av_init_packet(this);
		data = nullptr;
		size = 0;
	}
	CAVPacket2(size_t sz) : CAVPacket2()
	{
		if (sz > 0)
			av_new_packet(this, sz);
	}
	int grow_by(int by)
	{
		return av_grow_packet(this, by);
	}
	void shrink_to(int to)
	{
		av_shrink_packet(this, to);
	}
	~CAVPacket2(){ av_free_packet(this); }

	operator bool()const{ return data != nullptr; }
	//void free(){ av_free_packet(this); }
};
struct Data
{
	//boost::asio::mutable_buffer data;
	uint8_t data[8000];
	int size;
};
struct Buf
{
	uint8_t data[6000];
};
struct NetworkData
{
	std::vector<string> input_SDPs;
	std::vector<string> IPs;
	std::vector<int> my_ports;
	std::vector<int> remote_ports;
};