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
typedef std::shared_ptr<udp::socket> SHP_Socket;
#define my_
#define my2_
struct CAVPacket2 : AVPacket
{
	CAVPacket2() : AVPacket()
	{
		av_init_packet(this);
		data = nullptr;
		size = 0;
	}
	CAVPacket2(size_t sz) : CAVPacket2()
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
	~CAVPacket2(){ av_free_packet(this); }

	operator bool()const{ return data != nullptr; }
	//void free(){ av_free_packet(this); }
};
typedef shared_ptr<CAVPacket2> SHP_CAVPacket2;
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
	CRTPReceive(vector<string> SDPs, vector<string> IPs, vector<int> my_ports, vector<int> remote_ports)
	{ 
		tracks = my_ports.size();
		cout << "mix";
		IPs_ = IPs;
		my_ports_ = my_ports;
		remote_ports_ = remote_ports;
#ifdef my
		for (int i = 0; i < tracks; ++i)
		{
			SHP_Socket a;
			//boost::asio::ip::udp::endpoint sender(boost::asio::ip::address::from_string(IPs_[i]), remote_ports_[i]);
			a.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), my_ports_[i])));
			//boost::asio::socket_base::keep_alive option(true);
			//a->set_option(option);
			vecSock.push_back(a);
			//vecSender.push_back(sender);
		}
#endif
		/*boost::asio::ip::udp::endpoint sender1(boost::asio::ip::address::from_string("10.77.8.211"), remote_ports_[0]);
		//boost::asio::ip::udp::endpoint sender1(boost::asio::ip::address::from_string("10.77.7.19"), remote_ports_[0]);
		vecSender.push_back(sender1);
		boost::asio::ip::udp::endpoint sender2(boost::asio::ip::address::from_string("10.77.8.5"), remote_ports_[1]);
		//boost::asio::ip::udp::endpoint sender2(boost::asio::ip::address::from_string("10.77.7.19"), remote_ports_[1]);
		vecSender.push_back(sender2);
		boost::asio::ip::udp::endpoint sender3(boost::asio::ip::address::from_string("10.77.2.39"), remote_ports_[2]);
		//boost::asio::ip::udp::endpoint sender3(boost::asio::ip::address::from_string("10.77.7.19"), remote_ports_[2]);
		vecSender.push_back(sender3);*/
		cout << "\nsock";
		//pSocket_1.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), my_port[0])));
		//pSocket_2.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), my_port[1])));
		//pSocket_3.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), my_port[2])));
		init(SDPs); 
	}
	
	int process_all();
	boost::asio::io_service io_service_;
private:
	void loggit(string a);
	int FirstInit();
	int init(vector<string> input_SDPs);
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
#ifdef my2_
	vector<AVFormatContext *> out_ifcx;
	vector<AVCodecContext *> out_iccx;
#endif
	vector<AVFormatContext *> ifcx;
	vector<AVCodecContext *> iccx;

	//vector<AVFormatContext *> ifcx2;
	//vector<AVCodecContext *> iccx2;

	vector<SSource> afcx;

	vector<AVFilterGraph *> graphVec;
	vector<AVFilterContext *> sinkVec;

	int tracks = 3;

	//vector<string> IPS_;
	//vector<int> ports_;
	vector<SHP_Socket> vecSock;
	vector<udp::endpoint> vecSender;
	vector<string> IPs_;
	vector<int> my_ports_;
	vector<int> remote_ports_;
	uint8_t data_[65535];
	//vector<std::shared_ptr<ofstream>> file;
	ofstream ff1;
	ofstream ff2;
	ofstream ff3;
	vector<string> SDPs_;
	//SHP_CAVPacket2 shpPacket;
	//boost::scoped_ptr<udp::socket> pSocket_1;
	//boost::scoped_ptr<udp::socket> pSocket_2;
	//boost::scoped_ptr<udp::socket> pSocket_3;
};
typedef std::shared_ptr<CRTPReceive> SHP_CRTPReceive;
