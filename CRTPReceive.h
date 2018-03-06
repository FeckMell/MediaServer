#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "Functions.h"
#include "CMixInit.h"
#include "Structs.h"

//#include "Utils.h"
#define INPUT_SAMPLERATE     8000
//#define INPUT_SAMPLERATE     44100
#define INPUT_FORMAT         AV_SAMPLE_FMT_S16
#define INPUT_CHANNEL_LAYOUT AV_CH_LAYOUT_STEREO

/** The output bit rate in kbit/s */
#define OUTPUT_BIT_RATE 8000
/** The number of output channels */
#define OUTPUT_CHANNELS 1
/** The audio sample output format */
#define OUTPUT_SAMPLE_FORMAT AV_SAMPLE_FMT_S16

#define VOLUME_VAL 0.90
using boost::asio::ip::udp;
using namespace boost::asio;
using namespace std;
extern string DateStr;
extern Logger CLogger;

//struct Initing;
//struct CAVPacket;
//struct Data;
//struct NetworkData;
//struct RTP_struct;

using namespace std::chrono;
typedef std::shared_ptr<udp::socket> SHP_Socket;
typedef shared_ptr<CAVPacket> SHP_CAVPacket;

class CRTPReceive
{
public:
	CRTPReceive(NetworkData net)
	{ 
		received = 9999;
		net_ = net;
		process_all_finishing = false;
		Initer.reset(new CMixInit(net_));
		ext = Initer->data;
		reinit_sockets(false);
		outfile0.open("ConfResult0.wav", std::ofstream::binary);
		outfile1.open("ConfResult1.wav", std::ofstream::binary);
		outfile2.open("ConfResult2.wav", std::ofstream::binary);
		outfile3.open("ConfResult3.wav", std::ofstream::binary);

	}
	~CRTPReceive()
	{
		//out << "\n~CRTPReceive()";
		process_all_finishing = true;
		Initer.reset();
		//out << "\n~CRTPReceive()";
	}
	void add_track(NetworkData net);
	int process_all(NetworkData net);
	void receive(int i);
	void destroy_all();
	
private:
	boost::asio::io_service io_service_;
	/*helpers*/
	
	void loggit(string a, int thread);
	
	/*main activity*/
	int init_input_frame(AVFrame **frame);
	void init_packet(AVPacket *packet);
	int decode_audio_frame(AVFrame *frame, int *data_present, int i);
	int encode_audio_frame(AVFrame *frame, int *data_present, int i);
	
	void new_process(unsigned i);
	void add_missing_frame(int i, int j);
	void get_last_buffer_frame(AVFrame* frame, int i);
	void add_to_filter(int i);

	/*event handling*/
	void reinit_sockets(bool mode);
	void clear_memmory();

	Initing ext; 

	vector<RTP_struct> rtp2;
	vector<SHP_Socket> vecSock;
	vector<udp::endpoint> vecEndpoint;
	vector<Data> vecData;
	vector<boost::shared_ptr<boost::thread>> receive_threads;
	NetworkData net_;
	vector<AVFrame*> vecFrame;//TODO free
	//vector<std::mutex> vecMute;
	
	SHP_CMixInit Initer;

	bool process_all_finishing;
	int received;
	std::mutex  mutex_;
	std::ofstream outfile0;
	std::ofstream outfile1;
	std::ofstream outfile2;
	std::ofstream outfile3;
};
typedef std::shared_ptr<CRTPReceive> SHP_CRTPReceive;

