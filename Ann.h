#pragma once
#include "stdafx.h"
#include "Functions.h"
#include "Structs.h"
#include "Logger.h"
#include "ConfPoint.h"
extern Logger CLogger;
extern string DateStr;
extern string MusicPath;
using namespace std;
using namespace std::chrono;

class Ann
{
public:
	Ann(){ running = false; }
	Ann(SHP_CConfPoint Point);
	Ann(string SDP, int my_port, string CallID);
	void Send(string file);
	string CallID_;
	void Stop(){ running = false; if (type){ th->join(); } }
	int GetPort(){ return my_port_; }
private:
	boost::asio::io_service io_service_;

	void loggit(string a);
	void openFile(string filename);
	void openRTP();
	void Run();
	int encode_audio_frame(SHP_CAVFrame frame, int *data_present);
	int decode_audio_frame(SHP_CAVFrame frame, int *data_present);
	void freeall();
	void reinit();



	string remote_ip_;
	int remote_port_;
	int my_port_;
	string filename_;

	RTP_struct rtp_hdr;
	SHP_Socket sock;
	udp::endpoint endpt;
	SHP_CAVPacket2 left_data;

	AVFormatContext* ifcx = nullptr;
	AVFormatContext* out_ifcx = nullptr;

	AVCodecContext* iccx;
	AVCodecContext* out_iccx;

	AVOutputFormat *ofmt = NULL;
	AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;

	bool running;
	bool type=false;
	boost::shared_ptr<boost::thread> th;

};
typedef std::shared_ptr<Ann> SHP_Ann;