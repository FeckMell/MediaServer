#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "Functions.h"
#include "CMixInit.h"
#include "Structs.h"
#include "ConfPoint.h"

#define INPUT_SAMPLERATE     8000
#define INPUT_FORMAT         AV_SAMPLE_FMT_S16
#define INPUT_CHANNEL_LAYOUT AV_CH_LAYOUT_STEREO
#define OUTPUT_BIT_RATE 8000
#define OUTPUT_CHANNELS 1
#define OUTPUT_SAMPLE_FORMAT AV_SAMPLE_FMT_S16
#define VOLUME_VAL 0.90

using boost::asio::ip::udp;
using namespace boost::asio;
using namespace std;
using namespace std::chrono;

extern string DateStr;
extern Logger CLogger;



class CRTPReceive
{
public:
	CRTPReceive(vector<SHP_CConfPoint> callers, int ID); 

	void add_track(vector<SHP_CConfPoint> callers);
	void Freeze();
	void destroy_all();

private:
	/*helpers*/
	void Run_io();
	void receive_h(boost::system::error_code ec, size_t szPack, int i);

	void loggit(string a);
	
	/*main activity*/
	int process_all();
	void new_process();
	void receive();
	void CRTPReceive::receive2(int i);//debug

	int init_input_frame(AVFrame **frame);
	void init_packet(AVPacket *packet);
	int decode_audio_frame(AVFrame *frame, int *data_present, int i);
	int encode_audio_frame(AVFrame *frame, int *data_present, int i);
	
	
	void add_missing_frame(int i, int j);
	void get_last_buffer_frame(AVFrame* frame, int i);
	void add_to_filter(int i, AVFrame* frame);

	/*event handling*/
	void clear_memmory();

	vector<SHP_CConfPoint> callers_;
	vector<boost::shared_ptr<boost::thread>> receive_threads;

	Initing ext;
	SHP_CMixInit Initer;

	bool process_all_finishing;
	mutex  mutex_;

	int ID_;
	//DEBUG
	std::ofstream outfile0;
	std::ofstream outfile1;
	std::ofstream outfile2;
	std::ofstream outfile3;
};
typedef std::shared_ptr<CRTPReceive> SHP_CRTPReceive;

