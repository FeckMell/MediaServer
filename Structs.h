#pragma once
#include "stdafx.h"
#include <boost/circular_buffer.hpp>

typedef std::shared_ptr<udp::socket> SHP_Socket;

struct SSource
{
	std::vector<AVFilterContext *> src;
};
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
struct Initing
{
	std::vector<SSource> afcx;
	std::vector<AVFilterGraph *> graphVec;
	std::vector<AVFilterContext *> sinkVec;
};
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
struct SdpOpaque
{
	using Vector = std::vector<uint8_t>;
	Vector data;
	Vector::iterator pos;
};
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
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
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
struct RTP_struct
{
	RTP header;
	long TS;
	int amount;

	RTP_struct()
	{
		rtp_config();
	}

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
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
class CAVFrame
{
public:
	CAVFrame(){ frame = av_frame_alloc(); Empty = false; }
	CAVFrame(bool a){ frame = av_frame_alloc(); Empty = true; }
	~CAVFrame(){ av_frame_free(&frame); }

	AVFrame* get(){ return frame; }
	void free(){ av_frame_free(&frame); }

	bool empty(){ return Empty; }
private:
	bool Empty;
	AVFrame* frame;
};
typedef shared_ptr<CAVFrame> SHP_CAVFrame;
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
class CAVPacket
{
public:
	CAVPacket(){ av_init_packet(&packet); packet.data = nullptr; packet.size = 0; }
	CAVPacket(size_t sz){ if (sz > 0)av_new_packet(&packet, sz); }
	~CAVPacket(){ free(); }

	AVPacket* get(){ return &packet; }
	int size(){ return packet.size; }
	uint8_t* data(){ return packet.data; }
	void free(){ av_free_packet(&packet); }
	void make_size(int n){ packet.size = n; }
private:
	AVPacket packet;
};
typedef shared_ptr<CAVPacket> SHP_CAVPacket;
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
struct Data
{
	~Data(){ free(); }
	uint8_t* data = new uint8_t[8000];
	int size = 160;
	void free()
	{
		delete[] data;
	}
};
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
struct NetworkData
{
	~NetworkData(){ free(); }
	std::vector<std::string> input_SDPs;
	std::vector<std::string> IPs;
	std::vector<int> my_ports;
	std::vector<int> remote_ports;
	SHP_Socket Sock;
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
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
struct Config
{
	~Config()
	{
		MediaPath.~basic_string();
		IP.~basic_string();
	}

	string MediaPath="";
	string IP="";
	short int SIPport = 2429;
	short int MGCPport=2427;
	short int RTPport = 29000;
	int error=0;
};
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
class CThreadedCircular
{
public:
	CThreadedCircular() : buffer_(3), buffer2_(3){  }
	CThreadedCircular(size_t sz) : buffer_(sz), buffer2_(sz){  }
	CThreadedCircular(const CThreadedCircular &obj)
	{
		buffer_ = obj.buffer_;
		buffer2_ = obj.buffer2_;
		size_ = obj.size_;
	}
	~CThreadedCircular(){ free(); }
	//-*/----------------------------------------------------------------------
	void setAmount(int n)
	{
		size_ = n;
	}
	//-*/----------------------------------------------------------------------
	void push(SHP_CAVFrame val)
	{
		mutex_.lock();
		buffer_.push_back(val);
		std::vector<int> a;
		a.resize(size_, 0);
		buffer2_.push_back(a);
		mutex_.unlock();
	}
	//-*/----------------------------------------------------------------------
	SHP_CAVFrame pop(int i)
	{
		SHP_CAVFrame result;
		mutex_.lock();
		//if (buffer_.empty()) { result = std::make_shared<CAVFrame>(true); }//cout << "\nempty, size =" << buffer_.size(); }
		//else {result = buffer_.front(); }
		result = get_last_frame(i);
		mutex_.unlock();
		return result;
	}
	//-*/----------------------------------------------------------------------
	int size()
	{
		int result;
		mutex_.lock();
		result = buffer_.size();
		mutex_.unlock();
		return result;
	}
	//-*/----------------------------------------------------------------------
	void free()
	{
		mutex_.lock();
		buffer_.clear();
		buffer2_.clear();
		mutex_.unlock();
	}
	bool empty()
	{
		bool result;
		mutex_.lock();
		result = buffer_.empty();
		mutex_.unlock();
		return result;
	}
private:
	SHP_CAVFrame get_last_frame(int i)
	{
		if (buffer_.empty()) { return std::make_shared<CAVFrame>(true); }
		else
		{
			for (int j = 0; j < (int)buffer2_.size(); ++j)
			{
				if (buffer2_[j][i] == 0)
				{
					buffer2_[j][i] = 1;
					//cout << "\nframe i" << i << " j" << j;
					return buffer_[j];
				}
				//else{ cout << "\nused"<<i; }
			}
			//cout << "\nall used";
			return std::make_shared<CAVFrame>(true);
		}
	}
	int size_;
	std::mutex  mutex_;
	boost::circular_buffer<SHP_CAVFrame>	buffer_;
	boost::circular_buffer<std::vector<int>> buffer2_;
};
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
