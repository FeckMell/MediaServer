#pragma once
#include "stdafx.h"

class CAnnRQNT
{
public:
	CAnnRQNT(string filename, string IP, int my_port, int remote_port)
	{
		init();
	}
	
private:
	void loggit(string a);
	int init();
	int process_all();
	int open_input_file();
	int open_output_file();
	int decode_audio_frame(AVFrame *frame, int *data_present);
	int encode_audio_frame(AVFrame *frame, int *data_present);
	int init_input_frame(AVFrame **frame);
	void init_packet(AVPacket *packet);
	AVFormatContext *ifcx;
	AVCodecContext *iccx;
	AVFormatContext *out_ifcx;
	AVCodecContext *out_iccx;

	string filename;
	int my_port;
	int remote_port;
	
};
typedef std::shared_ptr<CAnnRQNT> SHP_CAnnRQNT;