#pragma once
#include "stdafx.h"

typedef std::shared_ptr<udp::socket> SHP_Socket;

struct SSource
{
	std::vector<AVFilterContext *> src;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
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
//-----------------------------------------------------------
//-----------------------------------------------------------
struct SdpOpaque
{
	using Vector = std::vector<uint8_t>;
	Vector data;
	Vector::iterator pos;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
struct RTP
{
	/* первый байт */
	uint8_t csrc_len : 4;//
	uint8_t extension : 1;//
	uint8_t padding : 1;//
	uint8_t version : 2;//
	/* второй байт */
	uint8_t payload_type : 7;//
	uint8_t marker : 1;//
	/* третий-четвертый байты */
	uint16_t seq_no;
	/* пятый-восьмой байты */
	uint32_t timestamp;//
	/* девятый-двенадцатый байт */
	uint32_t ssrc;//
};
//-----------------------------------------------------------
//-----------------------------------------------------------
struct RTP_struct
{
	RTP header;
	long TS;
	int amount;
	void rtp_config()
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
	void rtp_modify()
	{
		++this->amount;
		this->header.seq_no = htons(this->amount);
		this->header.timestamp = htonl(160 * this->amount);
	}
};
//-----------------------------------------------------------
//-----------------------------------------------------------
struct CAVPacket : AVPacket
{
	CAVPacket() : AVPacket()
	{
		av_init_packet(this);
		data = nullptr;
		size = 0;
	}
	CAVPacket(size_t sz) : CAVPacket()
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
	void free(){ av_free_packet(this); }
	~CAVPacket(){ av_free_packet(this); }

	operator bool()const{ return data != nullptr; }
	//void free(){ av_free_packet(this); }
};
typedef shared_ptr<CAVPacket> SHP_CAVPacket;
//-----------------------------------------------------------
//-----------------------------------------------------------
struct Data
{
	~Data(){ free(); }
	uint8_t* data = new uint8_t[8000];
	int size;
	void free()
	{
		delete[] data;
	}
};
//-----------------------------------------------------------
//-----------------------------------------------------------
struct NetworkData
{
	~NetworkData(){ free(); }
	std::vector<string> input_SDPs;
	std::vector<string> IPs;
	std::vector<int> my_ports;
	std::vector<int> remote_ports;
	void free()
	{
		for (unsigned i = 0; i < input_SDPs.size(); ++i)
		{
			input_SDPs[i].clear();
			IPs[i].clear();
		}
		input_SDPs.clear();
		IPs.clear();
		my_ports.clear();
		remote_ports.clear();

		input_SDPs.~vector();
		IPs.~vector();
		my_ports.~vector();
		remote_ports.~vector();
	}
};
//-----------------------------------------------------------
//-----------------------------------------------------------
struct Config
{
	~Config()
	{
		MediaPath.~basic_string();
		IP.~basic_string();
	}
	string MediaPath;
	string IP;
	short int port=2427;
	int error=0;
};
