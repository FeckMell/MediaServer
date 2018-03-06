#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "Functions.h"
#include "CMixInit.h"
#include "Structs.h"

#define INPUT_SAMPLERATE     8000
#define INPUT_FORMAT         AV_SAMPLE_FMT_S16
#define INPUT_CHANNEL_LAYOUT AV_CH_LAYOUT_STEREO
#define OUTPUT_BIT_RATE 8000
#define OUTPUT_CHANNELS 1
#define OUTPUT_SAMPLE_FORMAT AV_SAMPLE_FMT_S16
#define VOLUME_VAL 0.99

using boost::asio::ip::udp;
using namespace boost::asio;
using namespace std;

extern string DateStr;
extern Logger CLogger;

using namespace std::chrono;

class CRTPReceive
{
public:
	CRTPReceive(NetworkData net)
	{ 
		net_ = net;
		process_all_finishing = false;
		Initer.reset(new CMixInit(net_));
		ext = Initer->data;
		reinit_sockets(false);
		outfile0.open("ConfResult0.wav", std::ofstream::binary);
		outfile1.open("ConfResult1.wav", std::ofstream::binary);
		outfile2.open("ConfResult2.wav", std::ofstream::binary);
		outfile3.open("ConfResult3.wav", std::ofstream::binary);
		loggit("111", 0);

	}
	~CRTPReceive()
	{
		process_all_finishing = true;
		Initer.reset();
	}

	void add_track(NetworkData net);
	int process_all(NetworkData net);
	
	void destroy_all();
	
private:
	/*helpers*/
	void Run_io();
	void receive_h(boost::system::error_code ec, size_t szPack, int i);
	void receive_i(unsigned i);
	void loggit(string a, int thread);
	
	/*main activity*/
	void new_process();
	void receive();

	int init_input_frame(AVFrame **frame);
	void init_packet(AVPacket *packet);
	int decode_audio_frame(AVFrame *frame, int *data_present, int i);
	int encode_audio_frame(AVFrame *frame, int *data_present, int i);
	
	
	void add_missing_frame(int i, int j);
	void get_last_buffer_frame(AVFrame* frame, int i);
	void add_to_filter(int i, AVFrame* frame);

	/*event handling*/
	void reinit_sockets(bool mode);
	void clear_memmory();

	

	vector<RTP_struct> rtp2;
	vector<SHP_Socket> vecSock;
	vector<boost::shared_ptr<boost::thread>> receive_threads;
	vector<udp::endpoint> vecEndpoint;
	vector<CThreadedCircular> vecBuf;
	vector<Data> vecData;

	NetworkData net_;
	Initing ext;
	SHP_CMixInit Initer;

	bool process_all_finishing;
	std::mutex  mutex_;
	boost::asio::io_service io_service_;
	std::ofstream outfile0;
	std::ofstream outfile1;
	std::ofstream outfile2;
	std::ofstream outfile3;
};
typedef std::shared_ptr<CRTPReceive> SHP_CRTPReceive;

