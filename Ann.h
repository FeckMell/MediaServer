#include "stdafx.h"
#include "Functions.h"
#include "Structs.h"
typedef std::shared_ptr<udp::socket> SHP_Socket;
class Ann
{
public:
	Ann(string SDP, int my_port, string CallID, int ID);
	void Send(string file);
	string CallID_;
private:
	boost::asio::io_service io_service_;
	void openFile(string filename);
	void openRTP();
	void Run();
	int encode_audio_frame(AVFrame *frame, int *data_present);
	int decode_audio_frame(AVFrame *frame, int *data_present);
	void init_packet(AVPacket *packet);



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

	
	int ID_;

};
typedef std::shared_ptr<Ann> SHP_Ann;