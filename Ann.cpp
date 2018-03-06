#include "stdafx.h"
#include "Ann.h"
void Ann::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	string time = "time:";
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	time += DateStr
		+ "/" + boost::to_string(t->tm_hour) + ":" + boost::to_string(t->tm_min) + ":" + boost::to_string(t->tm_sec) + "/" 
		+ boost::to_string(t1.time_since_epoch().count() % 1000);
	CLogger.AddToLog(5, "\n" + time + " User=" + CallID_ + "       " + a);
}
Ann::Ann(string SDP, int my_port, string CallID)
{
	loggit("Ann construct");
	av_log_set_level(0);
	av_register_all();
	avcodec_register_all();
	avfilter_register_all();
	avformat_network_init();

	running = true;
	CallID_ = CallID;
	remote_ip_ = MakeRemoteIP(SDP);
	//out << "\nip=" << remote_ip_;
	remote_port_ = stoi(MakeRemotePort(SDP));
	//out << "     port=" << remote_port_;
	my_port_ = my_port;
	rtp_hdr.rtp_config();
	loggit("using remote_ip=" + remote_ip_ + "_ remote_port=" + boost::to_string(remote_port_) + "_ my_port=" + boost::to_string(my_port_)+"_");
	sock.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), my_port_)));
	endpt = udp::endpoint(boost::asio::ip::address::from_string(remote_ip_), remote_port_);
	left_data.reset(new CAVPacket(0));
	loggit("Ann construct DONE");
}

void Ann::openFile(string filename)
{
	int err = 0;
	
	err = avformat_open_input(&ifcx, filename.c_str(), NULL, NULL);
	loggit("Ann::openFile err=" + boost::to_string(err));
	err = avformat_find_stream_info(ifcx, NULL);
	loggit("Ann::openFile stream err=" + boost::to_string(err));
	//Выбор индекса потока
	/*for (unsigned i = 0; idxStream_ == -1 && i < ifcx->nb_streams; ++i)
	idxStream_ = ifcx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO
	? i : -1;*/
	AVCodec *input_codec = avcodec_find_decoder(ifcx->streams[0]->codec->codec_id);
	avcodec_open2(ifcx->streams[0]->codec, input_codec, nullptr);
	if (!ifcx->streams[0]->codec->channel_layout)
		ifcx->streams[0]->codec->channel_layout = av_get_default_channel_layout(ifcx->streams[0]->codec->channels);
	loggit("Ann::openFile DONE");
}
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
void Ann::Send(string file)
{
	loggit("Ann::Send");
	//out << "\n" + file;
	filename_ = file;
	openFile(file);
	openRTP();
	loggit("Ann::Send init done, calling RUN()");
	Run();
}
void Ann::Run()
{
	
	int data_present = 0;
	int i = 0;
	loggit("Ann::Run() DONE");
	while (running)
	{
		data_present = 0;
		AVFrame *frame = NULL;
		frame = av_frame_alloc();
		if (-1 == decode_audio_frame(frame, &data_present))
		{
			loggit("Ann::Run() stoped call freeall()");
			freeall();
			return;
		}
		encode_audio_frame(frame, &data_present);
		av_frame_free(&frame);
	}
	loggit("Ann::Run() stoped call freeall()");
	freeall();
}
int Ann::decode_audio_frame(AVFrame *frame, int *data_present)
{
	loggit("Ann::decode_audio_frame");
	AVPacket input_packet;
	SHP_CAVPacket send;
	int error;
	init_packet(&input_packet);
	if ((error = av_read_frame(ifcx, &input_packet)) < 0) 
	{
		loggit("Ann::decode_audio_frame error read");
		//out << "\nerrrrrrrr";
		return -1;
		/** If we are the the end of the file, flush the decoder below. */
		/*add end of file*/
	}

	send.reset(new CAVPacket(input_packet.size));
	memcpy(send->data, input_packet.data, input_packet.size);
	avcodec_decode_audio4(/*iccx*/ifcx->streams[0]->codec, frame, data_present, send.get());

	loggit("Ann::decode_audio_frame DONE data readed=" + boost::to_string(send->size));
	av_free_packet(&input_packet);
	send->free();
	return 0;
}
int Ann::encode_audio_frame(AVFrame *frame, int *data_present)
{
	loggit("Ann::encode_audio_frame");
	AVPacket output_packet;
	SHP_CAVPacket send;
	//	int error;
	init_packet(&output_packet);

	avcodec_encode_audio2(out_iccx, &output_packet, frame, data_present);
	if (left_data->size == 0)
	{
		int i = 0;
		while ((i + 1) * 160 <= output_packet.size)
		{
			send.reset(new CAVPacket(160 + 12));
			rtp_hdr.rtp_modify();
			memcpy(send->data, (uint8_t*)&rtp_hdr.header, 12);
			memcpy(send->data + 12, output_packet.data + i * 160, 160);
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
			loggit("senting bytes = " + boost::to_string(send->size));
			sock->send_to(boost::asio::buffer(send->data, send->size), endpt);
			send->free();
			++i;
		}
		//сохраняем остатки для следующего прогона
		left_data.reset(new CAVPacket(output_packet.size - i * 160));
		memcpy(left_data->data, output_packet.data + i * 160, output_packet.size - i * 160);
		loggit("data left = " + boost::to_string(left_data->size));
	}
	else
	{
		//копируем остатки
		send.reset(new CAVPacket(160 + 12));
		rtp_hdr.rtp_modify();
		memcpy(send->data, (uint8_t*)&rtp_hdr.header, 12);
		memcpy(send->data + 12, left_data->data, left_data->size);
		memcpy(send->data + 12 + left_data->size, output_packet.data, 160 - left_data->size);
		output_packet.size = output_packet.size - left_data->size;
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		loggit("senting bytes = " + boost::to_string(send->size));
		sock->send_to(boost::asio::buffer(send->data, send->size), endpt);
		send->free();
		//теперь уже новый пакет
		int i = 0;
		while ((i + 1) * 160 + (160 - left_data->size) <= output_packet.size)
		{
			send.reset(new CAVPacket(160 + 12));
			rtp_hdr.rtp_modify();
			memcpy(send->data, (uint8_t*)&rtp_hdr.header, 12);
			memcpy(send->data + 12, output_packet.data + i * 160 + (160 - left_data->size), 160);
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
			loggit("senting bytes = " + boost::to_string(send->size));
			sock->send_to(boost::asio::buffer(send->data, send->size), endpt);
			send->free();
			++i;
		}
		//копируем остатки
		left_data.reset(new CAVPacket(output_packet.size - i * 160));
		memcpy(left_data->data, output_packet.data + i * 160, output_packet.size - i * 160);
		loggit("data left = " + boost::to_string(left_data->size));
	}
	loggit("Ann::encode_audio_frame DONE");
	av_free_packet(&output_packet);
	return 0;
}
void Ann::init_packet(AVPacket *packet)
{
	av_init_packet(packet);
	/** Set the packet data and size so that it is recognized as being empty. */
	packet->data = NULL;
	packet->size = 0;
}
void Ann::freeall()
{
	left_data->free();
	sock->close();
	avformat_close_input(&ifcx);
	ifcx = NULL;
	avcodec_free_context(&out_iccx);
	avio_close(out_ifcx->pb);
	avformat_free_context(ifcx);
	loggit("All freed!");
}