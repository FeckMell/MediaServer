#pragma once
#include "stdafx.h"
#include "CRTPReceive.h"
#include <boost/thread.hpp>
using namespace std;
class CRTPReceive;


void CRTPReceive::loggit(string a)
{
	fprintf(FileLogMixer, ("\n" + a + "\n//-------------------------------------------------------------------").c_str());
	fflush(FileLogMixer);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int sdp_read1(void *opaque, uint8_t *buf, int size) /*noexcept*/
{
	assert(opaque);
	assert(buf);
	auto octx = static_cast<SdpOpaque*>(opaque);

	if (octx->pos == octx->data.end()) 
		return 0;
	
	auto dist = static_cast<int>(std::distance(octx->pos, octx->data.end()));
	auto count = std::min(size, dist);
	std::copy(octx->pos, octx->pos + count, buf);
	octx->pos += count;

	return count;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void worker_thread(CRTPReceive *a)
{
	a->io_service_.run();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	cout << "\nwaited!!!";
}
int CRTPReceive::FirstInit()
{
	loggit("int CRTPReceive::FirstInit()");
	av_log_set_level(AV_LOG_VERBOSE);
	av_register_all();
	avcodec_register_all();
	avfilter_register_all();
	avformat_network_init();
	for (int i = 0; i < tracks; ++i)
	{
		AVFormatContext *input_format_context = NULL;
		AVCodecContext *input_codec_context = NULL;
		AVFormatContext *output_format_context = NULL;
		AVCodecContext *output_codec_context = NULL;
		boost::thread Thread(worker_thread, this);
		Thread.detach();
		SSource a;

		for (int j = 0; j < tracks - 1; ++j)
		{
			AVFilterContext* src = NULL;
			a.src.push_back(src);
		}

		ifcx.push_back(input_format_context);
		iccx.push_back(input_codec_context);
		out_ifcx.push_back(output_format_context);
		out_iccx.push_back(output_codec_context);
		afcx.push_back(a);
	}
	loggit("int CRTPReceive::FirstInit() ENDED");
	return 1;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CRTPReceive::init(vector<string> input_SDPs)
{
	loggit("int CRTPReceive::init(vector<string> SDP)");
	int err;
	ff1.open("test_in.dat");
	string logSDP = "";
	string logSocket = "";
	//if (input_SDPs.size() != output_SDPs.size())
	//	assert(true);
	//else
	//	SetNumSources(input_SDPs.size());

	FirstInit();
	for (int i = 0; i < tracks; ++i)
	{
		logSDP += input_SDPs[i] + "\n";
		logSocket += "\nrtp://" + IPs_[i] + ":" + to_string(remote_ports_[i]) + " -> " + to_string(my_ports_[i]);
		//logAddr += output_SDPs[i] + "\n";
	}
	loggit("SDPs in filter:\n" + logSDP + "addresses:" + logSocket);
	
	for (int i = 0; i < tracks; ++i)
	{
		if (open_input_file(input_SDPs[i].c_str(), i) < 0)
		{
			loggit("Error while opening file " + to_string(i));
			system("pause");
		}
		av_dump_format(ifcx[i], 0, input_SDPs[i].c_str(), 0);
	}
	loggit("open input files DONE\nfor (int i = 0; i < tracks; ++i) init_filter_graph(i)");
	for (int i = 0; i < tracks; ++i)// вернуть
	{
		err = init_filter_graph(i);
		if (err < 0)
		{
			loggit("Init err =  " + to_string(err));
			system("pause");
		}
	}
	loggit("Initing filters DONE\n for (int i = 0; i < tracks; ++i) open_output_file");
	//for (int i = 0; i < tracks; ++i)
	for (int i = (tracks - 1); i > -1; --i)
	{
		//av_log(NULL, AV_LOG_INFO, "Output file : %s\n", output_SDPs[i].c_str());
		cout << "\nopen_output_file(output_SDPs[i].c_str(),i);" << i;
		char out[100];
		snprintf(out, sizeof(out), "output%d.wav", i);
		err = open_output_file(out, i);
		//cout << "\nopen_output_file " << i << ", " << output_SDPs[i];
		loggit("open output file err :  " + std::to_string(err));
		//av_dump_format(out_ifcx[i], 0, output_SDPs[i].c_str(), 1);
		
		if (write_output_file_header(out_ifcx[i]) < 0)
		{
			loggit("Error while writing header outputfile  " + std::to_string(err));
			//cout << "write_output_file_header(out_ifcx[i]) " << output_SDPs[i];
			//system("pause");
		}
	}
	loggit("for (int i = 0; i < tracks; ++i) open_output_file DONE\n init DONE");
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
char *const get_error_text(const int error)
{
	static char error_buffer[255];
	av_strerror(error, error_buffer, sizeof(error_buffer));
	return error_buffer;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CRTPReceive::sdp_open(AVFormatContext **pctx, const char *data, AVDictionary **options) /*noexcept*/
{
	loggit("int CRTPReceive::sdp_open");
	assert(pctx);
	*pctx = avformat_alloc_context();
	assert(*pctx);

	const size_t avioBufferSize = 4096;
	auto avioBuffer = static_cast<uint8_t*>(av_malloc(avioBufferSize));
	auto opaque = new SdpOpaque();
	opaque->data = SdpOpaque::Vector(data, data + strlen(data));
	opaque->pos = opaque->data.begin();

	auto pbctx = avio_alloc_context(avioBuffer, avioBufferSize, 0, opaque, &sdp_read1, nullptr, nullptr);
	assert(pbctx);
	(*pctx)->pb = pbctx;

	auto infmt = av_find_input_format("sdp");
	loggit("int CRTPReceive::sdp_open DONE");
	cout << "\n1.1";
	return avformat_open_input(pctx, "memory.sdp", /*nullptr*/infmt, nullptr/*options*/);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CRTPReceive::init_filter_graph(int ForClient)
{
	loggit("int CRTPReceive::init_filter_graph");
	AVFilterGraph   *filter_graph;
	AVFilterContext *mix_ctx;
	AVFilter        *mix_filter;
	AVFilterContext *abuffersink_ctx;
	AVFilter        *abuffersink;

	char args[512];
	int err;

	/* Создаем фильтр-граф, содержащий все фильтры */
	filter_graph = avfilter_graph_alloc();
	if (!filter_graph) 
	{
		loggit("Unable to create filter graph.");
		av_log(NULL, AV_LOG_ERROR, "Unable to create filter graph.\n");
		return AVERROR(ENOMEM);
	}

	/****** abuffer [ForClient][i] ********/
	for (int i = 0; i < tracks; ++i)
	{
		loggit("/****** abuffer [ForClient][i] ********/");
		/*for i==ForClient we dont set buffer*/
		if (i == ForClient) continue;// вернуть
		AVFilter *abuffer0;
		char arg[10];
		/* Создаем абуффер фильтр. он используется для "скармливания" информации в граф */
		
		abuffer0 = avfilter_get_by_name("abuffer");
		if (!abuffer0)
		{
			//string s(get_error_text(error));
			loggit("Could not find the abuffer filter.");
			av_log(NULL, AV_LOG_ERROR, "Could not find the abuffer filter.\n");
			return AVERROR_FILTER_NOT_FOUND;
		}
		/*источник буффера: раскодированные фреймы из декодера будут в здесь*/
		/* buffer audio source: the decoded frames from the decoder will be inserted here. */
		if (!iccx[i]->channel_layout)
		{
			iccx[i]->channel_layout = av_get_default_channel_layout(iccx[i]->channels);
		}

		snprintf(args, sizeof(args), "sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64,
			iccx[i]->sample_rate, av_get_sample_fmt_name(iccx[i]->sample_fmt), iccx[i]->channel_layout);
		snprintf(arg, sizeof(arg), "src%d-%d", ForClient, i);
		//разбиение индекса для SSource.
		if (i < ForClient)
			err = avfilter_graph_create_filter(&afcx[ForClient].src[i], abuffer0, arg, args, NULL, filter_graph);
		else
			err = avfilter_graph_create_filter(&afcx[ForClient].src[i - 1], abuffer0, arg, args, NULL, filter_graph);
		if (err < 0)
		{
			loggit("Cannot create audio buffer source");
			av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer source\n");
			return err;
		}
	}

	/****** amix ******* */
	/* Create mix filter. */
	loggit("/****** amix ******* */\n/* Create mix filter. */");
	mix_filter = avfilter_get_by_name("amix");
	if (!mix_filter) 
	{
		loggit("Could not find the mix filter.");
		av_log(NULL, AV_LOG_ERROR, "Could not find the mix filter.\n");
		return AVERROR_FILTER_NOT_FOUND;
	}

	char arg[10];
	snprintf(arg, sizeof(arg), "amix%d", ForClient);
	snprintf(args, sizeof(args), "inputs=%d", tracks - 1);

	err = avfilter_graph_create_filter(&mix_ctx, mix_filter, arg,
		args, NULL, filter_graph);

	if (err < 0)
	{
		loggit("Cannot create audio amix filter");
		av_log(NULL, AV_LOG_ERROR, "Cannot create audio amix filter\n");
		return err;
	}

	/* Finally create the abuffersink filter;* it will be used to get the filtered data out of the graph. */
	abuffersink = avfilter_get_by_name("abuffersink");
	if (!abuffersink)
	{
		loggit("Could not find the abuffersink filter.");
		av_log(NULL, AV_LOG_ERROR, "Could not find the abuffersink filter.\n");
		return AVERROR_FILTER_NOT_FOUND;
	}

	snprintf(arg, sizeof(arg), "sink%d", ForClient);
	abuffersink_ctx = avfilter_graph_alloc_filter(filter_graph, abuffersink, arg);
	if (!abuffersink_ctx)
	{
		loggit("Could not allocate the abuffersink instance.");
		av_log(NULL, AV_LOG_ERROR, "Could not allocate the abuffersink instance.\n");
		return AVERROR(ENOMEM);
	}

	/* Same sample fmts as the output file. */
	const enum AVSampleFormat Fmts[] = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
	err = av_opt_set_int_list(abuffersink_ctx, "sample_fmts", Fmts, AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN);

	char ch_layout[64];
	av_get_channel_layout_string(ch_layout, sizeof(ch_layout), 0, OUTPUT_CHANNELS);
	av_opt_set(abuffersink_ctx, "channel_layout", ch_layout, AV_OPT_SEARCH_CHILDREN);

	if (err < 0)
	{
		loggit("Could set options to the abuffersink instance." + err);
		av_log(NULL, AV_LOG_ERROR, "Could set options to the abuffersink instance.\n");
		return err;
	}

	err = avfilter_init_str(abuffersink_ctx, NULL);
	if (err < 0)
	{
		loggit("Could not initialize the abuffersink instance." + err);
		av_log(NULL, AV_LOG_ERROR, "Could not initialize the abuffersink instance.\n");
		return err;
	}

	loggit("/* Connect the filters; */");
	/* Connect the filters; */
	int indexx = 0;
	for (int i = 0; i < tracks - 1 ; ++i)
	{
		//if (i == ForClient) continue;
		err = avfilter_link(afcx[ForClient].src[i], 0, mix_ctx, indexx);
		if (err < 0)
		{
			loggit("Error connecting filters " + to_string(i));
			av_log(NULL, AV_LOG_ERROR, "Error connecting filters\n");
			return err;
		}
		++indexx;
	}
	err = avfilter_link(mix_ctx, 0, abuffersink_ctx, 0);
	if (err < 0) 
	{
		loggit("Error connecting filters" + to_string(err));
		av_log(NULL, AV_LOG_ERROR, "Error connecting filters\n");
		return err;
	}


	/* Configure the graph. */
	err = avfilter_graph_config(filter_graph, NULL);
	if (err < 0) 
	{
		string s(get_error_text(err));
		loggit("Error while configuring graph :" + s);
		av_log(NULL, AV_LOG_ERROR, "Error while configuring graph : %s\n", get_error_text(err));
		return err;
	}

	char* dump = avfilter_graph_dump(filter_graph, NULL);
	//loggit("Error while configuring graph :" + s);
	av_log(NULL, AV_LOG_ERROR, "Graph :\n%s\n", dump);

	graphVec.push_back(filter_graph);
	sinkVec.push_back(abuffersink_ctx);

	loggit("int CRTPReceive::init_filter_graph END");
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CRTPReceive::open_input_file(const char * SDP, int i)
{
	loggit("int CRTPReceive::open_input_file");
	AVCodec *input_codec;
	int error;

	/** Open the input file to read from it. */
	//cout << "\n1";
	error = sdp_open(&ifcx[i], SDP, nullptr);
	if (error < 0)
	{
		string s(get_error_text(error));
		loggit("Could not open input file (error" + s);
		av_log(NULL, AV_LOG_ERROR, "Could not open input file '%s' (error '%s')\n",
			SDP, get_error_text(error));
		ifcx[i] = NULL;
		return error;
	}
	//cout << "\n2";
	/** Get information on the input file (number of streams etc.). */
	/*if ((error = avformat_find_stream_info(ifcx[i], NULL)) < 0)
	{
		string s(get_error_text(error));
		loggit("Could not open find stream info (error" + s);
		av_log(NULL, AV_LOG_ERROR, "Could not open find stream info (error '%s')\n",
			get_error_text(error));
		avformat_close_input(&ifcx[i]);
		return error;
	}*/
	//cout << "\n3";
	/** Make sure that there is only one stream in the input file. */
	if ((ifcx[i])->nb_streams != 1)
	{
		loggit("Expected one audio input stream, but found " + (ifcx[i])->nb_streams);
		av_log(NULL, AV_LOG_ERROR, "Expected one audio input stream, but found %d\n",
			(ifcx[i])->nb_streams);
		avformat_close_input(&ifcx[i]);
		return AVERROR_EXIT;
	}
	//cout << "\n4";
	/** Find a decoder for the audio stream. */
	if (!(input_codec = avcodec_find_decoder((ifcx[i])->streams[0]->codec->codec_id)))
	{
		loggit("Could not find input codec");
		av_log(NULL, AV_LOG_ERROR, "Could not find input codec\n");
		avformat_close_input(&ifcx[i]);
		return AVERROR_EXIT;
	}
	//cout << "\n5";
	/** Open the decoder for the audio stream to use it later. */
	if ((error = avcodec_open2((ifcx[i])->streams[0]->codec, input_codec, NULL)) < 0)
	{
		string s(get_error_text(error));
		loggit("Could not open input codec (error " + s);
		av_log(NULL, AV_LOG_ERROR, "Could not open input codec (error '%s')\n",
			get_error_text(error));
		avformat_close_input(&ifcx[i]);
		return error;
	}

	/** Save the decoder context for easier access later. */
	iccx[i] = (ifcx[i])->streams[0]->codec;
	loggit("int CRTPReceive::open_input_file END");
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CRTPReceive::open_output_file(const char *filename, int i)
{
	loggit("int CRTPReceive::open_output_file");
	AVIOContext *output_io_context = NULL;
	AVStream *stream = NULL;
	AVCodec *output_codec = NULL;
	int error;

	/** Open the output file to write to it. */
	if ((error = avio_open(&output_io_context, filename,
		AVIO_FLAG_WRITE)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s' (error '%s')\n",
			filename, get_error_text(error));
		return error;
	}

	/** Create a new format context for the output container format. */
	if (!(out_ifcx[i] = avformat_alloc_context())) {
		av_log(NULL, AV_LOG_ERROR, "Could not allocate output format context\n");
		return AVERROR(ENOMEM);
	}

	/** Associate the output file (pointer) with the container format context. */
	(out_ifcx[i])->pb = output_io_context;

	/** Guess the desired container format based on the file extension. */
	if (!((out_ifcx[i])->oformat = av_guess_format(NULL, filename,
		NULL))) {
		av_log(NULL, AV_LOG_ERROR, "Could not find output file format\n");
		goto cleanup;
	}

	av_strlcpy((out_ifcx[i])->filename, filename,
		sizeof((out_ifcx[i])->filename));

	/** Find the encoder to be used by its name. */
	if (!(output_codec = avcodec_find_encoder((ifcx[i])->streams[0]->codec->codec_id))) {
		av_log(NULL, AV_LOG_ERROR, "Could not find an PCM encoder.\n");
		goto cleanup;
	}

	/** Create a new audio stream in the output file container. */
	if (!(stream = avformat_new_stream(out_ifcx[i], output_codec))) {
		av_log(NULL, AV_LOG_ERROR, "Could not create new stream\n");
		error = AVERROR(ENOMEM);
		goto cleanup;
	}

	/** Save the encoder context for easiert access later. */
	out_iccx[i] = stream->codec;

	/**
	* Set the basic encoder parameters.
	*/
	(out_iccx[i])->channels = OUTPUT_CHANNELS;
	(out_iccx[i])->channel_layout = av_get_default_channel_layout(OUTPUT_CHANNELS);
	(out_iccx[i])->sample_rate = iccx[i]->sample_rate;
	(out_iccx[i])->sample_fmt = AV_SAMPLE_FMT_S16;
	//(*output_codec_context)->bit_rate       = input_codec_context->bit_rate;

	av_log(NULL, AV_LOG_INFO, "output bitrate %d\n", (out_iccx[i])->bit_rate);

	/**
	* Some container formats (like MP4) require global headers to be present
	* Mark the encoder so that it behaves accordingly.
	*/
	if ((out_ifcx[i])->oformat->flags & AVFMT_GLOBALHEADER)
		(out_iccx[i])->flags |= CODEC_FLAG_GLOBAL_HEADER;

	/** Open the encoder for the audio stream to use it later. */
	if ((error = avcodec_open2(out_iccx[i], output_codec, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Could not open output codec (error '%s')\n",
			get_error_text(error));
		goto cleanup;
	}

	return 0;

cleanup:
	avio_close((out_ifcx[i])->pb);
	avformat_free_context(out_ifcx[i]);
	out_ifcx[i] = NULL;
	return error < 0 ? error : AVERROR_EXIT;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
/** Initialize one audio frame for reading from the input file */
int CRTPReceive::init_input_frame(AVFrame **frame)
{
	if (!(*frame = av_frame_alloc())) 
	{
		loggit("Could not allocate input frame");
		av_log(NULL, AV_LOG_ERROR, "Could not allocate input frame\n");
		return AVERROR(ENOMEM);
	}
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
/** Initialize one data packet for reading or writing. */
void CRTPReceive::init_packet(AVPacket *packet)
{
	av_init_packet(packet);
	/** Set the packet data and size so that it is recognized as being empty. */
	packet->data = NULL;
	packet->size = 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
/** Decode one audio frame from the input file. */
int CRTPReceive::decode_audio_frame(AVFrame *frame, int *data_present, int *finished,int i)
{
	cout << "\n1";
	loggit("int CRTPReceive::decode_audio_frame");
	/** Packet used for temporary storage. */
	AVPacket input_packet;
	int error;
	init_packet(&input_packet);
	SHP_CAVPacket2 shpPacket;
	uint8_t data_[8100];
	cout << " 2";
	boost::asio::ip::udp::endpoint sender(boost::asio::ip::address::from_string(IPs_[i]), remote_ports_[i]);
	cout << " 3";
	SHP_Socket a;
	a.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), my_ports_[i])));
	int szPack;// = vecSock[i]->receive_from(boost::asio::buffer(data_, 2048), sender) - 12;
	szPack = /*vecSock[i]*/a->receive_from(boost::asio::buffer(data_, 8100), sender);
	if (szPack > 12)
	{
		cout << " size=" << szPack;
		cout << " ----4";
		shpPacket.reset(new CAVPacket2(szPack));
		cout << " 5";
		memcpy(shpPacket->data, data_ + 12, szPack - 12);
		cout << " 6";
	}

	/*vecSock[i]->async_receive_from(
		boost::asio::buffer(data, 2048), sender,
		[this](boost::system::error_code ec, std::size_t bytes_recvd)
	{
		size_t RTP_HEADER_SIZE = 12;
		if (bytes_recvd > RTP_HEADER_SIZE)
		{
			//cout << data_;
			ff1.write((const char*)data, strlen((const char*)data));
			flush(ff1);
			const size_t szPack = bytes_recvd - RTP_HEADER_SIZE;
			shpPacket.reset(new CAVPacket2(szPack));
			memcpy(shpPacket->data, data + RTP_HEADER_SIZE, szPack);
		}
	});*/
	//input_packet = shpPacket.get();
	// Read one audio frame from the input file into a temporary packet. 
	/*if ((error = av_read_frame(ifcx[i], &input_packet)) < 0) 
	{
		// If we are the the end of the file, flush the decoder below. 
		if (error == AVERROR_EOF)
			*finished = 1;
		else 
		{
			string s(get_error_text(error));
			loggit("Could not read frame (error " + s);
			av_log(NULL, AV_LOG_ERROR, "Could not read frame (error '%s')\n",
				get_error_text(error));
			return error;
		}
	}*/
	/**
	* Decode the audio frame stored in the temporary packet.
	* The input audio stream decoder is used to do this.
	* If we are at the end of the file, pass an empty packet to the decoder
	* to flush it.
	*/
	cout << " 7";
	if ((error = avcodec_decode_audio4(iccx[i], frame, data_present, shpPacket.get())) < 0)
	{
		string s(get_error_text(error));
		loggit("Could not decode frame (error" + s);
		av_log(NULL, AV_LOG_ERROR, "Could not decode frame (error '%s')\n",
			get_error_text(error));
		av_free_packet(&input_packet);
		return error;
	}
	cout << " 8";
	if (*finished && *data_present)
		*finished = 0;
	av_free_packet(&input_packet);
	loggit("int CRTPReceive::decode_audio_frame END");
	//free(data_);
	cout << " 9";
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
/** Encode one frame worth of audio to the output file. */
int CRTPReceive::encode_audio_frame(AVFrame *frame, int *data_present, int i)
{
	loggit("int CRTPReceive::encode_audio_frame");
	/** Packet used for temporary storage. */
	AVPacket output_packet;
	int error;
	init_packet(&output_packet);

	/**
	* Encode the audio frame and store it in the temporary packet.
	* The output audio stream encoder is used to do this.
	*/
	if ((error = avcodec_encode_audio2(out_iccx[i], &output_packet,
		frame, data_present)) < 0) 
	{
		string s(get_error_text(error));
		loggit("Could not encode frame (error " + s);
		av_log(NULL, AV_LOG_ERROR, "Could not encode frame (error '%s')\n",
			get_error_text(error));
		av_free_packet(&output_packet);
		return error;
	}

	/** Write one audio frame from the temporary packet to the output file. */
	if (*data_present) {
		vecSock[i]->send_to(boost::asio::buffer(frame->buf, strlen((const char*)frame->buf)), udp::endpoint(boost::asio::ip::address::from_string(IPs_[i]), remote_ports_[i]));
		
		/*vecSock[i]->async_send_to(
			boost::asio::buffer(frame->buf, strlen((const char*)frame->buf)), udp::endpoint(boost::asio::ip::address::from_string(IPs_[i]), remote_ports_[i]),
			[](boost::system::error_code ec, std::size_t bytes_recvd)
		{
			//cout << "sent\n";
		});*/
		/*if ((error = av_interleaved_write_frame(out_ifcx[i], &output_packet)) < 0)
		{
			string s(get_error_text(error));
			loggit("Could not write frame (error " + s);
			av_log(NULL, AV_LOG_ERROR, "Could not write frame (error '%s')\n",
				get_error_text(error));
			av_free_packet(&output_packet);
			return error;
		}*/

		av_free_packet(&output_packet);
	}
	loggit("int CRTPReceive::encode_audio_frame END");
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CRTPReceive::process_all()
{
	loggit("int CRTPReceive::process_all() initing");
	int ret = 0;
	int data_present = 0;
	int finished = 0;
	int total_out_samples = 0;
	int nb_finished = 0;

	vector<int> input_finished;
	vector<int> input_to_read;
	vector<int> total_samples;
	for (int i = 0; i < tracks - 1; ++i)
	{
		input_finished.push_back(0);
		input_to_read.push_back(1);
		total_samples.push_back(0);
	}

	loggit("int CRTPReceive::process_all() initing DONE");
	cout << "process_all";
	while (nb_finished < tracks)
	{
		loggit(" in the start of while (nb_finished < tracks)");
		int data_present_in_graph = 0;

		for (int i = 0; i < tracks; ++i)
		{
			loggit(" in the start of for (int i = 0; i < tracks; ++i) i=" + to_string(i));
			//if (input_finished[i] || input_to_read[i] == 0) continue; //не вернуть
			input_to_read[i] = 0;
			AVFrame *frame = NULL;

			if (init_input_frame(&frame) > 0) { std::cout << "\n 2.1"; goto end; }
			/** Decode one frame worth of audio samples. */
			if ((ret = decode_audio_frame(frame, &data_present, &finished, i)))
			{
				loggit("if ((ret = decode_audio_frame(frame, ifcx[i], iccx[i], &data_present, &finished))) FAILED");
				goto end;
			}
			/**
			* If we are at the end of the file and there are no more samples
			* in the decoder which are delayed, we are actually finished.
			* This must not be treated as an error.
			*/
			if (finished && !data_present)
			{
				input_finished[i] = 1;
				nb_finished++;
				ret = 0;
				loggit("Write NULL frame. Input finished: " + to_string(i));
				av_log(NULL, AV_LOG_INFO, "Input n°%d finished. Write NULL frame \n", i);
				//каждому клиенту в соответствующий буффер даем фрейм
				for (int j = 0; j < tracks; ++j) //вернуть
				{
					if (i == j) continue;
					if (j < i)
						ret = av_buffersrc_write_frame(afcx[j].src[i - 1], NULL);
					else
						ret = av_buffersrc_write_frame(afcx[j].src[i], NULL);
					if (ret < 0)
					{
						loggit("Error writing EOF null frame for input " + to_string(i));
						av_log(NULL, AV_LOG_ERROR, "Error writing EOF null frame for input %d\n", i);
						goto end;
					}
				}
			}
			else if (data_present)
			{
				/** If there is decoded data, convert and store it */
				/* push the audio data from decoded frame into the filtergraph */
				//каждому клиенту в соответствующий буффер даем фрейм
				for (int j = 0; j < tracks; ++j) //вернуть
				{
					if (i == j) continue;

					if (j < i)
						ret = av_buffersrc_write_frame(afcx[j].src[i - 1], frame);
					else
						ret = av_buffersrc_write_frame(afcx[j].src[i], frame);
					if (ret < 0)
					{
						loggit("Error writing EOF null frame for input " + to_string(i));
						av_log(NULL, AV_LOG_ERROR, "Error writing EOF null frame for input %d\n", i);
						goto end;
					}
				 }
			}

			av_frame_free(&frame);

			data_present_in_graph = data_present | data_present_in_graph;
		}
		if (data_present_in_graph)
		{
			/* pull filtered audio from the filtergraph */
			for (int k = 0; k < tracks; ++k)/*****/ //вернуть
			{
				AVFrame *filt_frame = av_frame_alloc();
				while (1)
				{
					ret = av_buffersink_get_frame(sinkVec[k], filt_frame);
					if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
					{
						for (int i = 0; i < tracks - 1; i++)
						{
							if (av_buffersrc_get_nb_failed_requests(afcx[k].src[i]) > 0)
							{
								input_to_read[i] = 1;
								loggit("Need to read input " + to_string(i));
							}
						}
						break;
					}
					if (ret < 0){ std::cout << "\n 2.5"; goto end; }
					loggit("remove samples from sink ( Hz, time=, ttime=)\n");
					if (ret = encode_audio_frame(filt_frame, &data_present, k)< 0)
					{ 
						loggit("if (ret = encode_audio_frame(filt_frame, out_ifcx[k], out_iccx[k], &data_present)< 0) FAILED");
						goto end; 
					}
					av_frame_unref(filt_frame);
				}/**/
				av_frame_free(&filt_frame);
			}/*****/
		}
		else
		{
			loggit("No data in graph\n");
			av_log(NULL, AV_LOG_INFO, "No data in graph\n");
			for (int i = 0; i < tracks; i++)
			{
				input_to_read[i] = 1;
			}
		}
	}
	loggit("Process all END");
	return 0;

end:

	if (ret < 0 && ret != AVERROR_EOF) 
	{
		loggit("av_log(NULL, AV_LOG_ERROR, Error occurred : \n, av_err2str(ret));");
		printf("av_log(NULL, AV_LOG_ERROR, Error occurred : \n, av_err2str(ret));");
		system("pause");
		exit(1);
	}
	system("pause");
	exit(2);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
/** Write the header of the output file container. */
int CRTPReceive::write_output_file_header(AVFormatContext *output_format_context)
{
	loggit("int CRTPReceive::write_output_file_header");
	int error;
	if ((error = avformat_write_header(output_format_context, NULL)) < 0) 
	{
		string s(get_error_text(error));
		loggit("Could not write output file header (error " + s);
		av_log(NULL, AV_LOG_ERROR, "Could not write output file header (error '%s')\n",
			get_error_text(error));
		return error;
	}
	loggit("int CRTPReceive::write_output_file_header END");
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
/** Write the trailer of the output file container. */
int CRTPReceive::write_output_file_trailer(AVFormatContext *output_format_context)
{
	loggit("int CRTPReceive::write_output_file_trailer");
	int error;
	if ((error = av_write_trailer(output_format_context)) < 0) 
	{
		string s(get_error_text(error));
		loggit("Could not write output file trailer (error " + s);
		av_log(NULL, AV_LOG_ERROR, "Could not write output file trailer (error '%s')\n",
			get_error_text(error));
		return error;
	}
	loggit("int CRTPReceive::write_output_file_trailer END");
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------