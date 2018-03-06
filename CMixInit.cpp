#pragma once
#include "stdafx.h"
#include "CMixInit.h"
#define newout

int sdp_read2(void *opaque, uint8_t *buf, int size) /*noexcept*/
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
void CMixInit::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	string time = "";
	time += DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(GetTickCount() % 1000);
	CLogger.AddToLog(4, "\n" + time + "       " + a + "\n//-------------------------------------------------------------------");
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CMixInit::FirstInit()
{
	loggit("int CMixInit::FirstInit()");
	av_log_set_level(0);
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
		SSource a;

		for (int j = 0; j < tracks - 1; ++j)
		{
			AVFilterContext* src = NULL;
			a.src.push_back(src);
		}
		data.ifcx.push_back(input_format_context);
		data.iccx.push_back(input_codec_context);
		data.out_ifcx.push_back(output_format_context);
		data.out_iccx.push_back(output_codec_context);
		data.afcx.push_back(a);
	}
	loggit("int CMixInit::FirstInit() ENDED");
	return 1;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CMixInit::init(vector<string> input_SDPs)
{
	loggit("int CMixInit::init(vector<string> SDP)");
	loggit("sizes =" + to_string(data.afcx.size()) + to_string(data.ifcx.size()));
	int err;
	string logSDP = "";
	string logSocket = "";
	FirstInit();
	for (int i = 0; i < tracks; ++i)
	{
		logSDP += input_SDPs[i] + "\n";
		logSocket += "\nrtp://" + net_.IPs[i] + ":" + to_string(net_.remote_ports[i]) + " -> " + to_string(net_.my_ports[i]);
	}
	loggit("SDPs in filter:\n" + logSDP + "addresses:" + logSocket);

	for (int i = 0; i < tracks; ++i)
	{
		if (open_input_file(input_SDPs[i].c_str(), i) < 0)
		{
			loggit("Error while opening file " + to_string(i));
			system("pause");
		}
		//av_dump_format(data.ifcx[i], 0, input_SDPs[i].c_str(), 0);
		char out[100];
		//snprintf(out, sizeof(out), "output%d.wav", i);
		err = open_output_file(out, i);
		

		/*if (write_output_file_header(data.out_ifcx[i]) < 0)
		{
			loggit("Error while writing header outputfile  " + boost::to_string(err));
			system("pause");
		}*/
		loggit("Opening input and output for i = " + to_string(i) + "good");
	}

	for (int i = 0; i < tracks; ++i)
	{
		err = init_filter_graph(i);
		if (err < 0)
		{
			loggit("Init err =  " + to_string(err) + "for i= " + to_string(i));
			system("pause");
		}
	}
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CMixInit::write_output_file_header(AVFormatContext *output_format_context)
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
int CMixInit::write_output_file_trailer(AVFormatContext *output_format_context)
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
int CMixInit::sdp_open(AVFormatContext **pctx, const char *data, AVDictionary **options) /*noexcept*/
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

	auto pbctx = avio_alloc_context(avioBuffer, avioBufferSize, 0, opaque, &sdp_read2, nullptr, nullptr);
	assert(pbctx);
	(*pctx)->pb = pbctx;

	auto infmt = av_find_input_format("sdp");
	loggit("int CRTPReceive::sdp_open DONE");
	return avformat_open_input(pctx, "memory.sdp", /*nullptr*/infmt, nullptr/*options*/);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CMixInit::init_filter_graph(int ForClient)
{
	loggit("int CRTPReceive::init_filter_graph");
	AVFilter        *mix_filter;
	AVFilterContext *mix_ctx;
	AVFilter        *abuffersink;
	AVFilterContext *abuffersink_ctx;
	AVFilterGraph   *filter_graph;
	char args[512];
	char arg[10];
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
		/*for i==ForClient we dont set buffer*/
		if (i == ForClient) continue;// вернуть
		AVFilter *abuffer0;
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
		if (!data.iccx[i]->channel_layout)
		{
			data.iccx[i]->channel_layout = av_get_default_channel_layout(data.iccx[i]->channels);

		}
		snprintf(args, sizeof(args), "sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64,
			data.iccx[i]->sample_rate, av_get_sample_fmt_name(data.iccx[i]->sample_fmt), data.iccx[i]->channel_layout);
		//snprintf(arg, sizeof(arg), "src%d-%d", ForClient, i);
		snprintf(arg, sizeof(arg), "src");
		//разбиение индекса для SSource.
		if (i < ForClient)
		{
			err = avfilter_graph_create_filter(&data.afcx[ForClient].src[i], abuffer0, arg, args, NULL, filter_graph);
		}
		else
		{
			err = avfilter_graph_create_filter(&data.afcx[ForClient].src[i - 1], abuffer0, arg, args, NULL, filter_graph);
		}

		if (err < 0)
		{
			loggit("Cannot create audio buffer source");
			av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer source\n");
			return err;
		}
	}

	/****** amix ******* */
	/* Create mix filter. */
	mix_filter = avfilter_get_by_name("amix");
	if (!mix_filter)
	{
		loggit("Could not find the mix filter.");
		av_log(NULL, AV_LOG_ERROR, "Could not find the mix filter.\n");
		return AVERROR_FILTER_NOT_FOUND;
	}

	//snprintf(arg, sizeof(arg), "amix%d", ForClient);
	snprintf(arg, sizeof(arg), "amix");
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

	//snprintf(arg, sizeof(arg), "sink%d", ForClient);
	snprintf(arg, sizeof(arg), "sink");
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

	/* Connect the filters; */
	int indexx = 0;
	for (int i = 0; i < tracks - 1; ++i)
	{
		//if (i == ForClient) continue;
		err = avfilter_link(data.afcx[ForClient].src[i], 0, mix_ctx, i);
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
	av_log(NULL, AV_LOG_ERROR, "Graph :\n%s\n", dump);

	data.graphVec.push_back(filter_graph);
	data.sinkVec.push_back(abuffersink_ctx);

	loggit("int CRTPReceive::init_filter_graph END");
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CMixInit::open_input_file(const char * SDP, int i)
{
	loggit("int CRTPReceive::open_input_file");
	AVCodec *input_codec;
	int error;

	/** Open the input file to read from it. */
	error = sdp_open(&data.ifcx[i], SDP, nullptr);
	if (error < 0)
	{
		string s(get_error_text(error));
		loggit("Could not open input file (error: " + s + ")");
		av_log(NULL, AV_LOG_ERROR, "Could not open input file '%s' (error '%s')\n",
			SDP, get_error_text(error));
		data.ifcx[i] = NULL;
		return error;
	}
	/** Get information on the input file (number of streams etc.). */
	/*if ((error = avformat_find_stream_info(ifcx[i], NULL)) < 0)
	{
	string s(get_error_text(error));
	av_log(NULL, AV_LOG_ERROR, "Could not open find stream info (error '%s')\n",
	get_error_text(error));
	avformat_close_input(&ifcx[i]);
	return error;
	}*/
	/** Make sure that there is only one stream in the input file. */
	if ((data.ifcx[i])->nb_streams != 1)
	{
		loggit("Expected one audio input stream, but found " + (data.ifcx[i])->nb_streams);
		av_log(NULL, AV_LOG_ERROR, "Expected one audio input stream, but found %d\n",
			(data.ifcx[i])->nb_streams);
		avformat_close_input(&data.ifcx[i]);
		return AVERROR_EXIT;
	}
	/** Find a decoder for the audio stream. */
	if (!(input_codec = avcodec_find_decoder((data.ifcx[i])->streams[0]->codec->codec_id)))
	{
		loggit("Could not find input codec");
		av_log(NULL, AV_LOG_ERROR, "Could not find input codec\n");
		avformat_close_input(&data.ifcx[i]);
		return AVERROR_EXIT;
	}
	/** Open the decoder for the audio stream to use it later. */
	if ((error = avcodec_open2((data.ifcx[i])->streams[0]->codec, input_codec, NULL)) < 0)
	{
		string s(get_error_text(error));
		loggit("Could not open input codec (error " + s);
		av_log(NULL, AV_LOG_ERROR, "Could not open input codec (error '%s')\n",
			get_error_text(error));
		avformat_close_input(&data.ifcx[i]);
		return error;
	}

	/** Save the decoder context for easier access later. */
	data.iccx[i] = (data.ifcx[i])->streams[0]->codec;
	loggit("int CRTPReceive::open_input_file END");


	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CMixInit::open_output_file(const char *filename, int i)
{
#ifndef newout
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
	if (!(data.out_ifcx[i] = avformat_alloc_context())) {
		av_log(NULL, AV_LOG_ERROR, "Could not allocate output format context\n");
		return AVERROR(ENOMEM);
	}

	/** Associate the output file (pointer) with the container format context. */
	(data.out_ifcx[i])->pb = output_io_context;

	/** Guess the desired container format based on the file extension. */
	if (!((data.out_ifcx[i])->oformat = av_guess_format(NULL, filename,
		NULL))) {
		av_log(NULL, AV_LOG_ERROR, "Could not find output file format\n");
		goto cleanup;
	}

	av_strlcpy((data.out_ifcx[i])->filename, filename,
		sizeof((data.out_ifcx[i])->filename));

	/** Find the encoder to be used by its name. */
	if (!(output_codec = avcodec_find_encoder((data.ifcx[i])->streams[0]->codec->codec_id))) {
		av_log(NULL, AV_LOG_ERROR, "Could not find an PCM encoder.\n");
		goto cleanup;
	}

	/** Create a new audio stream in the output file container. */
	if (!(stream = avformat_new_stream(data.out_ifcx[i], output_codec))) {
		av_log(NULL, AV_LOG_ERROR, "Could not create new stream\n");
		error = AVERROR(ENOMEM);
		goto cleanup;
	}

	/** Save the encoder context for easiert access later. */
	data.out_iccx[i] = stream->codec;

	/**
	* Set the basic encoder parameters.
	*/
	(data.out_iccx[i])->channels = OUTPUT_CHANNELS;
	(data.out_iccx[i])->channel_layout = av_get_default_channel_layout(OUTPUT_CHANNELS);
	(data.out_iccx[i])->sample_rate = data.iccx[i]->sample_rate;
	(data.out_iccx[i])->sample_fmt = AV_SAMPLE_FMT_S16;
	//(*output_codec_context)->bit_rate       = input_codec_context->bit_rate;

	av_log(NULL, AV_LOG_INFO, "output bitrate %d\n", (data.out_iccx[i])->bit_rate);

	/**
	* Some container formats (like MP4) require global headers to be present
	* Mark the encoder so that it behaves accordingly.
	*/
	/*if ((data.out_ifcx[i])->oformat->flags & AVFMT_GLOBALHEADER)
		(data.out_iccx[i])->flags |= CODEC_FLAG_GLOBAL_HEADER;*/

	/** Open the encoder for the audio stream to use it later. */
	if ((error = avcodec_open2(data.out_iccx[i], output_codec, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Could not open output codec (error '%s')\n",
			get_error_text(error));
		goto cleanup;
	}
	return 0;

cleanup:
	avio_close((data.out_ifcx[i])->pb);
	avformat_free_context(data.out_ifcx[i]);
	data.out_ifcx[i] = NULL;
	return error < 0 ? error : AVERROR_EXIT;

#else
	auto strRTP = str(boost::format("rtp://%1%:%2%?localport=%3%") % net_.IPs[i]/*remote_ip_*/ % net_.remote_ports[i] /*remote_port_*/ % (net_.my_ports[i] - 1000)/*(my_port_ - 1000)*/);
	//out << "\nrtp to: " << strRTP;
	avformat_alloc_output_context2(&data.out_ifcx[i], nullptr, "rtp", strRTP.c_str());
	avio_open(&data.out_ifcx[i]->pb, strRTP.c_str(), AVIO_FLAG_WRITE);

	AVCodecID idCodec = AV_CODEC_ID_PCM_ALAW;
	AVCodec *output_codec = avcodec_find_encoder(idCodec);

	auto strmOut = avformat_new_stream(data.out_ifcx[i], output_codec);
	strmOut->time_base = { 1, 8000 };
	data.out_iccx[i] = strmOut->codec;

	data.out_iccx[i]->channels = 1;
	data.out_iccx[i]->channel_layout = av_get_default_channel_layout(data.out_iccx[i]->channels);
	data.out_iccx[i]->sample_fmt = output_codec->sample_fmts[0];
	data.out_iccx[i]->sample_rate = 8000;
	data.out_iccx[i]->bit_rate = 8000;
	data.out_iccx[i]->time_base = { 1, data.out_iccx[i]->sample_rate };

	avcodec_open2(strmOut->codec, output_codec, nullptr);
#endif // !newout
return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CMixInit::FreeSockFFmpeg()
{
	loggit("Destroing filter");//
	for (auto &e : data.graphVec)
	{
		avfilter_graph_free(&e);
	}
	for (auto &e : data.ifcx)
	{ 
		avformat_close_input(&e);
		avformat_free_context(e);
		e = NULL;
	}
	for (auto &e : data.out_iccx)
	{
		avcodec_close(e);//
		avcodec_free_context(&e);
	}
	for (auto &e : data.out_ifcx)
	{
		avio_close(e->pb);
	}

	for (int i = 0; i < tracks; ++i)
	{
		//avcodec_close(data.out_iccx[i]);//
		//avformat_close_input(&data.ifcx[i]);
		//data.ifcx[i] = NULL;
		
		//avcodec_free_context(&data.out_iccx[i]);
		//avio_close(data.out_ifcx[i]->pb);
		//avformat_free_context(data.ifcx[i]);
		

		net_.input_SDPs[i].clear();
		net_.IPs[i].clear();
		//---------------------------------------------------------------
		//avformat_free_context(data.out_ifcx[i]);
		//avcodec_close(data.iccx[i]);
		//avcodec_close(data.out_iccx[i]);
		//avcodec_free_context(&data.iccx[i]);
	}
	net_.input_SDPs.clear();
	net_.IPs.clear();
	net_.my_ports.clear();
	net_.remote_ports.clear();
	loggit("Destroing filter DONE!");//
}

