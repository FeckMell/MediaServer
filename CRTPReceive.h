#pragma once
#include "stdafx.h"
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
//asio::io_service service2456;

struct SdpOpaque
{
	using Vector = std::vector<uint8_t>; Vector data; Vector::iterator pos;
};

struct SSource
{
	vector<AVFilterContext *> src;
};

class CRTPReceive
{
public:
	CRTPReceive(vector<string> input_SDPs, vector<string> output_SDPs)
	{ 
		//asio::io_service service2;
		//io_service_(service2);
		//pSocket_1.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), my_port[0])));
		//pSocket_2.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), my_port[1])));
		//pSocket_3.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), my_port[2])));
		init(input_SDPs, output_SDPs); 
	}
	
	int process_all();
	//boost::asio::io_service io_service_;
private:
	void loggit(string a);
	int FirstInit();
	int init(vector<string> input_SDPs, vector<string> output_SDPs);
	void SetNumSources(int i) { tracks = i; }

	int write_output_file_header(AVFormatContext *output_format_context);
	int write_output_file_trailer(AVFormatContext *output_format_context);
	int init_input_frame(AVFrame **frame);
	void init_packet(AVPacket *packet);
	int sdp_open(AVFormatContext **pctx, const char *data, AVDictionary **options);
	int init_filter_graph(int ForClient);
	int open_input_file(const char *sdp_string, int i);
	int open_output_file(const char *filename, int i);
	int decode_audio_frame(AVFrame *frame, int *data_present, int *finished, int i);
	int encode_audio_frame(AVFrame *frame, int *data_present, int i);
	//int CRTPReceive::open_output_file2(const char *filename, int i);

	vector<AVFormatContext *> out_ifcx;
	vector<AVCodecContext *> out_iccx;

	vector<AVFormatContext *> ifcx;
	vector<AVCodecContext *> iccx;

	vector<SSource> afcx;

	vector<AVFilterGraph *> graphVec;
	vector<AVFilterContext *> sinkVec;

	int tracks = 3;

	//vector<string> IPS_;
	//vector<int> ports_;
	
	//boost::scoped_ptr<udp::socket> pSocket_1;
	//boost::scoped_ptr<udp::socket> pSocket_2;
	//boost::scoped_ptr<udp::socket> pSocket_3;
};
typedef std::shared_ptr<CRTPReceive> SHP_CRTPReceive;