#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "Functions.h"
#include "CFilterInit.h"
#include "Structs.h"
#include "ConfPoint.h"



using boost::asio::ip::udp;
using namespace boost::asio;
using namespace std;
using namespace std::chrono;

extern string DateStr;
extern Logger CLogger;



class ConfAudio
{
public:
	ConfAudio(vector<SHP_CConfPoint> callers, int ID); 

	void add_track(vector<SHP_CConfPoint> callers);
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
	void ConfAudio::receive2(int i);//debug

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
	SHP_CFilterInit Initer;

	bool process_all_finishing;
	mutex  Mut_io;
	mutex  Mut_pr;


	int ID_;
	//DEBUG
	/*std::ofstream outfile0;
	std::ofstream outfile1;
	std::ofstream outfile2;
	std::ofstream outfile3;

	std::ofstream outfile00;
	std::ofstream outfile11;
	std::ofstream outfile22;
	std::ofstream outfile33;

	std::ofstream outfile000;
	std::ofstream outfile111;
	std::ofstream outfile222;
	std::ofstream outfile333;*/
};
typedef std::shared_ptr<ConfAudio> SHP_ConfAudio;

