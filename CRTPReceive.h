#pragma once
#include "stdafx.h"
#include "CMixInit.h"
#include "Structs.h"
#include <fstream> 
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
extern FILE *FileLogMixer;
struct Initing;
struct SSource;
struct CAVPacket2;
struct Data;
struct Buf;

typedef std::shared_ptr<udp::socket> SHP_Socket;
typedef shared_ptr<CAVPacket2> SHP_CAVPacket2;

class CRTPReceive
{
public:
	CRTPReceive(vector<string> input_SDPs, vector<string> IPs, vector<int> my_ports, vector<int> remote_ports)
	{ 
		process_all_finished = false;
		tracks = my_ports.size();
		Init.reset(new CMixInit(input_SDPs, IPs, my_ports, remote_ports));
		reinit_sockets(my_ports,false);
		IPs_ = IPs;
		my_ports_ = my_ports;
		remote_ports_ = remote_ports;
		process_all_stopper = true;
		ext = Init->data;	
		sockets_stoped = 0;
	}
	~CRTPReceive()
	{
		cout << "~CRTPReceive()";
		process_all_stopper = false;

		for (int i = 0; i < tracks; ++i)
		{
			vecSock[i]->close();
		}
		Init.reset();
		cout << "~CRTPReceive()";
	}
	void add_track(vector<string> input_SDPs, vector<string> IPs, vector<int> my_ports, vector<int> remote_ports);
	int process_all();
	void receive(int i);
	void destroy_all();
	boost::asio::io_service io_service_;
private:
	void loggit(string a);
	void reinit_sockets(vector<int> my_ports, bool mode);

	int init_input_frame(AVFrame **frame);
	void init_packet(AVPacket *packet);
	int decode_audio_frame(AVFrame *frame, int *data_present, int *finished, int i);
	int encode_audio_frame(AVFrame *frame, int *data_present, int i);
	int encode_audio_frame_file(AVFrame *frame, int *data_present, int i);
	


	int tracks;
	Initing ext; 

	vector<SHP_CAVPacket2> rtp;
	vector<SHP_Socket> vecSock;
	vector<SHP_Socket> vecSock2;
	vector<udp::endpoint> vecEndpoint;
	vector<string> IPs_;
	vector<int> my_ports_;
	vector<int> remote_ports_;
	uint8_t data[8000];
	vector<Data> vecData;
	vector<Data> vecData2;
	vector<boost::shared_ptr<boost::thread>> receive_threads;
	vector<std::chrono::high_resolution_clock::time_point> time_start_receive;
	//std::chrono::high_resolution_clock::time_point time_start_receive;
	
	vector<bool> skipper;
	bool process_all_stopper = true;
	bool process_all_finished;
	SHP_CAVPacket2 send;
	SHP_CMixInit Init;
	//int average = 360;
	int sockets_stoped=0;
};
typedef std::shared_ptr<CRTPReceive> SHP_CRTPReceive;

