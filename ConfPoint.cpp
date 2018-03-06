#include "stdafx.h"
#include "ConfPoint.h"


//-*/-------------------------------------------------------------------------
void CConfPoint::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	string result = DateStr + "/" + boost::to_string(t->tm_hour) + ":" + boost::to_string(t->tm_min) + ":" + boost::to_string(t->tm_sec) + "/" + boost::to_string(t1.time_since_epoch().count() % 1000);
	result += " ID=" + CallID_ + " thread=" + boost::to_string(std::this_thread::get_id()) + "      ";
	CLogger.AddToLog(8, "\n" + result + a);
}
//-*/-------------------------------------------------------------------------
CConfPoint::CConfPoint(string SDPff, string SDPfc, string CallID, int port, asio::io_service& io_service) :
CallID_(CallID), my_port_(port), SDP_for_client(SDPfc), io_service_(io_service)
{
	rtp.rtp_config();
	if (SDPff != "")
	{
		SDP_ = SDPff;
		ModifySDP();
		remote_port_ = stoi(MakeRemotePort(SDPff));
		remote_ip_ = MakeRemoteIP(SDPff);
		if (open_input() < 0) error = -1;
		if (open_output() < 0) error = -1;
		if (error != -1) { mode = true; need_free = true; }
	}
}
//-*/-------------------------------------------------------------------------
string CConfPoint::ModifyPoint(string SDPff)
{
	if (SDP_ == "")
	{
		SDP_ = SDPff;
		ModifySDP();
		remote_port_ = stoi(MakeRemotePort(SDPff));
		remote_ip_ = MakeRemoteIP(SDPff);
		if (open_input() < 0) error = -1;
		if (open_output() < 0) error = -1;
		if (error != -1) { mode = true; need_free = true; }
		return "";
	}
	else
	{
		ChangeMode(SDPff);
		return SDP_for_client;
	}
}
//-*/-------------------------------------------------------------------------
//-*/-------------------------------------------------------------------------
//-*/-------------------------------------------------------------------------
void CConfPoint::ChangeMode(string SDP)
{
	std::size_t fd1, fd2;
	fd1 = SDP.find("inactive");
	fd2 = SDP_for_client.find("sendrecv");
	if ((fd1 != std::string::npos) && (fd2 != std::string::npos))
	{
		SDP_for_client.replace(fd2, 8, "inactive");
		SDP_for_client = ChangeVersion(SDP_for_client);
		mode = false;
	}
	fd1 = SDP.find("sendrecv");
	fd2 = SDP_for_client.find("inactive");
	if ((fd1 != std::string::npos) && (fd2 != std::string::npos))
	{
		SDP_for_client.replace(fd2, 8, "sendrecv");
		SDP_for_client = ChangeVersion(SDP_for_client);
		mode = true;
	}
}
//-*/-------------------------------------------------------------------------
string CConfPoint::ChangeVersion(string SDP)
{
	std::size_t fd1;
	fd1 = SDP.find("o=");
	if (fd1 != std::string::npos)
	{
		fd1 = SDP.find(" ", fd1 + 1);
		if (fd1 != std::string::npos)
		{
			fd1 = SDP.find(" ", fd1 + 1);
			if (fd1 != std::string::npos)
			{
				string temp = SDP.substr(fd1 + 1, SDP.find(" ", fd1 + 1));
				int temp2 = stoi(temp) + 1;
				SDP.replace(fd1, SDP.find(" ", fd1 + 1) - fd1 + 1, " " + std::to_string(temp2) + " ");
			}
		}
	}
	return SDP;
}
//-*/-------------------------------------------------------------------------
void CConfPoint::ModifySDP()
{
	std::size_t found = SDP_.find("m=audio");
	if (found != std::string::npos)
		SDP_ = SDP_.replace(found + 8, SDP_.find(" ", found + 10) - found - 8, std::to_string(my_port_ - 1000));
	string rtp0 = "a=rtpmap:0";
	string rtp18 = "a=rtpmap:18";
	string rtp101 = "a=rtpmap:101";
	string rtpavp = "RTP/AVP";
	string fmtp = "a=fmtp:";
	// delete rtp 18, 101, 0
	found = SDP_.find(rtp18);
	if (found != std::string::npos)
		SDP_.replace(found, SDP_.find("\n", found + 1) - found + 1, "");

	found = SDP_.find(rtp101);
	if (found != std::string::npos)
		SDP_.replace(found, SDP_.find("\n", found + 1) - found + 1, "");

	found = SDP_.find(rtp0);
	if (found != std::string::npos)
		SDP_.replace(found, SDP_.find("\n", found + 1) - found + 1, "");

	found = SDP_.find(fmtp);
	while (found != std::string::npos)
	{
		SDP_.replace(found, SDP_.find("\n", found + 1) - found + 1, "");
		found = SDP_.find(fmtp);
	}
}
//-*/-------------------------------------------------------------------------
//-*/-------------------------------------------------------------------------
//-*/-------------------------------------------------------------------------
int CConfPoint::open_output()
{

	auto strRTP = str(boost::format("rtp://%1%:%2%?localport=%3%") % remote_ip_ % remote_port_ % (my_port_ - 2000));
	avformat_alloc_output_context2(&out_ifcx, nullptr, "rtp", strRTP.c_str());
	avio_open(&out_ifcx->pb, strRTP.c_str(), AVIO_FLAG_WRITE);

	AVCodecID idCodec = AV_CODEC_ID_PCM_ALAW;
	//AVCodecID idCodec = ifcx->streams[0]->codec->codec_id;
	AVCodec *output_codec = avcodec_find_encoder(idCodec);

	auto strmOut = avformat_new_stream(out_ifcx, output_codec);
	strmOut->time_base = { 1, 8000 };
	out_iccx = strmOut->codec;

	out_iccx->channels = 1;
	out_iccx->channel_layout = av_get_default_channel_layout(out_iccx->channels);
	out_iccx->sample_fmt = output_codec->sample_fmts[0];
	out_iccx->sample_rate = 8000;
	out_iccx->bit_rate = 8000;
	out_iccx->time_base = { 1, out_iccx->sample_rate };

	avcodec_open2(strmOut->codec, output_codec, nullptr);
	/////////////////////////////////////////////
	Sock.reset(new boost::asio::ip::udp::socket(io_service_));
	Sock->open(udp::v4());
	Sock->set_option(boost::asio::ip::udp::socket::reuse_address(true));
	Sock->bind(udp::endpoint(udp::v4(), my_port_));

	//Endpoint = udp::endpoint(boost::asio::ip::address::from_string(remote_ip_), remote_port_);
	/////////////////////////////////////////////
	return 0;
}
//-*/-------------------------------------------------------------------------
int CConfPoint::open_input()
{
	loggit("open_input");
	AVCodec *input_codec;
	int error;
	loggit("SDP:\n" + SDP_);
	error = sdp_open(&ifcx, SDP_.c_str(), nullptr);
	if (error < 0)
	{
		string s(get_error_text(error));
		loggit("Could not open input file (error: " + s + ")");
		av_log(NULL, AV_LOG_ERROR, "Could not open input file '%s' (error '%s')\n",
			SDP_, get_error_text(error));
		ifcx = NULL;
		return error;
	}
	if ((ifcx)->nb_streams != 1)
	{
		loggit("Expected one audio input stream, but found " + (ifcx)->nb_streams);
		av_log(NULL, AV_LOG_ERROR, "Expected one audio input stream, but found %d\n",
			(ifcx)->nb_streams);
		avformat_close_input(&ifcx);
		return AVERROR_EXIT;
	}
	
	if (!(input_codec = avcodec_find_decoder((ifcx)->streams[0]->codec->codec_id)))
	{
		loggit("Could not find input codec");
		av_log(NULL, AV_LOG_ERROR, "Could not find input codec\n");
		avformat_close_input(&ifcx);
		return AVERROR_EXIT;
	}
	if ((error = avcodec_open2((ifcx)->streams[0]->codec, input_codec, NULL)) < 0)
	{
		string s(get_error_text(error));
		loggit("Could not open input codec (error " + s);
		av_log(NULL, AV_LOG_ERROR, "Could not open input codec (error '%s')\n",
			get_error_text(error));
		avformat_close_input(&ifcx);
		return error;
	}

	iccx = (ifcx)->streams[0]->codec;
	loggit("int ConfAudio::open_input_file END");

	return 0;
}
//-*/-------------------------------------------------------------------------
int CConfPoint::sdp_open(AVFormatContext **pctx, const char *data, AVDictionary **options) /*noexcept*/
{
	assert(pctx);
	*pctx = avformat_alloc_context();
	assert(*pctx);

	const size_t avioBufferSize = 4096;
	auto avioBuffer = static_cast<uint8_t*>(av_malloc(avioBufferSize));
	auto opaque = new SdpOpaque();
	opaque->data = SdpOpaque::Vector(data, data + strlen(data));
	opaque->pos = opaque->data.begin();

	auto pbctx = avio_alloc_context(avioBuffer, avioBufferSize, 0, opaque, &sdp_read, nullptr, nullptr);
	assert(pbctx);
	(*pctx)->pb = pbctx;

	auto infmt = av_find_input_format("sdp");
	return avformat_open_input(pctx, "memory.sdp", infmt, nullptr);
}
//-*/-------------------------------------------------------------------------
void CConfPoint::free()
{
	if (need_free)
	{
		loggit("free:");
		mode = false;
		loggit("Sock->close();");
		Sock->close();
		loggit("avformat_close_input(&ifcx);");
		//
		avformat_close_input(&ifcx);
		loggit("avformat_free_context(ifcx);");
		avformat_free_context(ifcx);
		loggit("ifcx = NULL;");
		ifcx = NULL;
		loggit("avcodec_close(out_iccx);//iccx?");
		//
		avcodec_close(out_iccx);//iccx?
		loggit("avcodec_free_context(&out_iccx);//iccx?");
		avcodec_free_context(&out_iccx);//iccx?
		loggit("out_iccx = NULL;");
		out_iccx = NULL;//iccx?
		loggit("avio_close(out_ifcx->pb);");
		//
		avio_close(out_ifcx->pb);
		loggit("free: DONE");
	}
}
//-*/-------------------------------------------------------------------------
//-*/-------------------------------------------------------------------------
//-*/-------------------------------------------------------------------------
//-*/-------------------------------------------------------------------------