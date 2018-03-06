#include "stdafx.h"
#include "Ann.h"

void Ann::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	steady_clock::time_point t1 = steady_clock::now();
	string result = DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	result += " ID=" + CallID_ + " thread=" + boost::to_string(this_thread::get_id()) + "      ";
	CLogger->AddToLog(5, "\n" + result + a);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Ann::Ann(string SDP, int my_port, string CallID)
{
	loggit("Ann construct");

	running = true;
	CallID_ = CallID;
	remote_ip_ = MakeRemoteIP(SDP);
	remote_port_ = stoi(MakeRemotePort(SDP));
	my_port_ = my_port;
	rtp_hdr.rtp_config();
	loggit("using remote_ip=" + remote_ip_ + "_ remote_port=" + boost::to_string(remote_port_) + "_ my_port=" + boost::to_string(my_port_)+"_");

	sock.reset(new boost::asio::ip::udp::socket(io_service_));
	sock->open(udp::v4());
	sock->set_option(boost::asio::ip::udp::socket::reuse_address(true));
	sock->bind(udp::endpoint(udp::v4(), my_port_));

	endpt = udp::endpoint(boost::asio::ip::address::from_string(remote_ip_), remote_port_);
	left_data.reset(new CAVPacket());
	loggit("Ann construct DONE");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Ann::Ann(SHP_CConfPoint Point)
{
	type = true;
	running = true;
	CallID_ = Point->CallID_;
	loggit("Ann from Conf");
	remote_ip_ = Point->remote_ip_;
	remote_port_ = Point->remote_port_;
	my_port_ = Point->my_port_;
	loggit("using remote_ip=" + remote_ip_ + "_ remote_port=" + boost::to_string(remote_port_) + "_ my_port=" + boost::to_string(my_port_) + "_");
	sock = Point->Sock;
	endpt = Point->Endpoint;
	left_data.reset(new CAVPacket());
	filename_ = "";
#ifdef WIN32
	openFile(MusicPath + "\\music_alaw.wav");
#endif
#ifdef __linux__
	openFile(MusicPath + "/music_alaw.wav");
#endif
	out_iccx = Point->out_iccx;
	out_ifcx = Point->out_ifcx;
	th.reset(new boost::thread(&Ann::Run, this));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::Send(string file)
{
	loggit("Ann::Send " + file);
	filename_ = file;
	openFile(file);
	openRTP();

	loggit("Ann::Send init done, calling RUN()");
	Run();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::openFile(string filename)
{
	int err = 0;

	err = avformat_open_input(&ifcx, filename.c_str(), NULL, NULL);
	loggit("Ann::openFile err=" + boost::to_string(err));
	err = avformat_find_stream_info(ifcx, NULL);
	loggit("Ann::openFile stream err=" + boost::to_string(err));
	AVCodec *input_codec = avcodec_find_decoder(ifcx->streams[0]->codec->codec_id);
	avcodec_open2(ifcx->streams[0]->codec, input_codec, nullptr);
	if (!ifcx->streams[0]->codec->channel_layout)
		ifcx->streams[0]->codec->channel_layout = av_get_default_channel_layout(ifcx->streams[0]->codec->channels);
	loggit("Ann::openFile DONE");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::openRTP()
{
	loggit("Ann::openRTP()");
	auto strRTP = str(boost::format("rtp://%1%:%2%?localport=%3%") % remote_ip_ %remote_port_ % (my_port_ - 1000));
	//out << "\nrtp to: " << strRTP;
	avformat_alloc_output_context2(&out_ifcx, nullptr, "rtp", strRTP.c_str());
	avio_open(&out_ifcx->pb, strRTP.c_str(), AVIO_FLAG_WRITE);

	AVCodecID idCodec = AV_CODEC_ID_PCM_ALAW;
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
	loggit("Ann::openRTP() DONE");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::Run()
{
	loggit("Ann::Run()");
	int data_present = 0;
	//int i = 0;
	while (running)
	{
		data_present = 0;
		SHP_CAVFrame frame=std::make_shared<CAVFrame>();
		if (-1 == decode_audio_frame(frame, &data_present))
		{
			loggit("Ann::Run() stoped call reinit()");
			reinit();
			return;
		}
		encode_audio_frame(frame, &data_present);
	}
	loggit("Ann::Run() stoped call freeall()");
	freeall();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int Ann::decode_audio_frame(SHP_CAVFrame frame, int *data_present)
{
	int error;
	SHP_CAVPacket input_packet = std::make_shared<CAVPacket>();
	if ((error = av_read_frame(ifcx, input_packet->get())) < 0)
	{
		loggit("Ann::decode_audio_frame error read");
		return -1;
	}
	avcodec_decode_audio4(ifcx->streams[0]->codec, frame->get(), data_present, input_packet->get()); 
	return 0;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int Ann::encode_audio_frame(SHP_CAVFrame frame, int *data_present)
{
	//loggit("Ann::encode_audio_frame");
	SHP_CAVPacket output_packet = std::make_shared<CAVPacket>();


	avcodec_encode_audio2(out_iccx, output_packet->get(), frame->get(), data_present);
	if (left_data->size() == 0)
	{
		int i = 0;
		while ((i + 1) * 160 <= output_packet->size())
		{
			SHP_CAVPacket send = std::make_shared<CAVPacket>(172);
			rtp_hdr.rtp_modify();
			memcpy(send->data(), (uint8_t*)&rtp_hdr.header, 12);
			memcpy(send->data() + 12, output_packet->data() + i * 160, 160);
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
			//loggit("senting bytes = " + boost::to_string(send->size()));
			sock->send_to(boost::asio::buffer(send->data(), send->size()), endpt);
			++i;
		}
		//сохраняем остатки для следующего прогона
		left_data.reset(new CAVPacket(output_packet->size() - i * 160));
		memcpy(left_data->data(), output_packet->data() + i * 160, output_packet->size() - i * 160);
		//loggit("data left = " + boost::to_string(left_data->size()));
	}
	else
	{
		//копируем остатки
		SHP_CAVPacket send = std::make_shared<CAVPacket>(172);
		rtp_hdr.rtp_modify();
		memcpy(send->data(), (uint8_t*)&rtp_hdr.header, 12);
		memcpy(send->data() + 12, left_data->data(), left_data->size());
		memcpy(send->data() + 12 + left_data->size(), output_packet->data(), 160 - left_data->size());
		output_packet->make_size(output_packet->size() - left_data->size());
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		//loggit("senting bytes = " + boost::to_string(send->size()));
		sock->send_to(boost::asio::buffer(send->data(), send->size()), endpt);
		//теперь уже новый пакет
		int i = 0;
		while ((i + 1) * 160 + (160 - left_data->size()) <= output_packet->size())
		{
			send.reset(new CAVPacket(172));
			rtp_hdr.rtp_modify();
			memcpy(send->data(), (uint8_t*)&rtp_hdr.header, 12);
			memcpy(send->data() + 12, output_packet->data() + i * 160 + (160 - left_data->size()), 160);
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
			//loggit("senting bytes = " + boost::to_string(send->size()));
			sock->send_to(boost::asio::buffer(send->data(), send->size()), endpt);
			++i;
		}
		//копируем остатки
		left_data.reset(new CAVPacket(output_packet->size() - i * 160));
		memcpy(left_data->data(), output_packet->data() + i * 160, output_packet->size() - i * 160);
		//loggit("data left = " + boost::to_string(left_data->size()));
	}
	//loggit("Ann::encode_audio_frame DONE");
	return 0;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::freeall()
{
	left_data->free();
	avformat_close_input(&ifcx);
	avformat_free_context(ifcx);
	ifcx = NULL;

	if (!type)
	{
		loggit("freeall for Ann");
		sock->close();
		avcodec_free_context(&out_iccx);
		avio_close(out_ifcx->pb);
	}
	else
	{
		th->~thread();
		loggit("freeall from Conf");
	}

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Ann::reinit()
{
	left_data->free();
	avformat_close_input(&ifcx);
	ifcx = NULL;
	avcodec_free_context(&out_iccx);
	avio_close(out_ifcx->pb);
	avformat_free_context(ifcx);
	openFile(filename_);
	openRTP();
	loggit("Ann::Send init done, calling RUN()");
	Run();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
