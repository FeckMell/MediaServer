#pragma once
#include "stdafx.h"
#include "DestFusion.h"


enum{ RTP_OUT_RATE = 8000 };
//-----------------------------------------------------------------------
int CDestFusion::remSrcRef(SHP_ISrcFusion shpSrc, string CallID)
{
	assert(shpSrc);
	RETERR_ON_FALSE(gl_abuffer,
		AVERROR_FILTER_NOT_FOUND, "Could not find the abuffer filter");
	const AVCodecContext* pCodecCtx = shpSrc->CodecCTX();
	TSrcRef srcRef;
	srcRef.shpSrc = shpSrc;
	srcRef.CallID = CallID;
	//printf("DESTROY GRAPH\n");
	/*srcRef->*/avfilter_free(ctxMix_);
	/*srcRef->*/avfilter_free(ctxSink_);
	//av_dump_format(ctxFormat_, 0, "memory.sdp", 0);
	filtGraf_.DestroyGraph();
	//cou << "\n    remove from: ";
	/*for (auto &e : cllSrcRefs_)
		cou <<" "<< e.CallID;*/
	cllSrcRefs_.erase(std::remove(cllSrcRefs_.begin(), cllSrcRefs_.end(), srcRef), cllSrcRefs_.end());

	return m_lastError;
}
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
int CDestFusion::addSrcRef(SHP_ISrcFusion shpSrc, string CallID)
{
	assert(shpSrc);
	//cout << "\naddSrcRef 1";
	/* Create the abuffer filter;
	* it will be used for feeding the data into the graph. */
	RETERR_ON_FALSE(gl_abuffer, 
		AVERROR_FILTER_NOT_FOUND, "Could not find the abuffer filter");
	//cout << "\naddSrcRef 2";
	static auto frmtFilt = 
		boost::format("sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64);

	//cout << "\naddSrcRef 3";
	const AVCodecContext* pCodecCtx = shpSrc->CodecCTX();

	TSrcRef srcRef;
	srcRef.shpSrc = shpSrc;
	srcRef.CallID = CallID;
	//cout << "\naddSrcRef 4";
	RETURN_AVERROR(
		avfilter_graph_create_filter(&srcRef.pctxFilter, gl_abuffer,
			str(boost::format("%1% (%2%)") %  shpSrc->Name() % cllSrcRefs_.size()
				).c_str(),
			str(frmtFilt
				% pCodecCtx->sample_rate
				% av_get_sample_fmt_name(pCodecCtx->sample_fmt)
				% pCodecCtx->channel_layout
				).c_str(),
			nullptr,
			filtGraf_),
		"Cannot create audio buffer source"
		);//stoped here 
	//cout << "\naddSrcRef 5";
	cllSrcRefs_.push_back(srcRef);
	return m_lastError;
}
//-----------------------------------------------------------------------
int CDestFusion::openRTP(const TRTP_Dest& rtpdest)
{
	//cout << "\n RTP OPENED";
	_cleanup();
	//cout << "\n888888888888888888888888888888888888888888888888888888888888888888\n\n";
	//cout << "rtpdest.strAddr=" << rtpdest.strAddr << "rtpdest.portDest=" << rtpdest.portDest << "rtpdest.portSrc=" << rtpdest.portSrc;
	//cout << "\n\n888888888888888888888888888888888888888888888888888888888888888888\n";
	isRTP_ = true;
	ptimeRTP_ = rtpdest.ptimeRTP;

	const auto strRTP = str(boost::format("rtp://%1%:%2%?localport=%3%")
		% rtpdest.strAddr % rtpdest.portDest %  rtpdest.portSrc);
	
	RETURN_AVERROR(
		avformat_alloc_output_context2(&ctxFormat_, nullptr, "rtp", strRTP.c_str()),
		boost::format("Cannot open RTP output context for %1%") % strRTP);
	
	RETURN_AVERROR(
		avio_open(&ctxFormat_->pb, strRTP.c_str(), AVIO_FLAG_WRITE),
		boost::format("avio_open error on %1%") % strRTP);
	
	const AVCodecID idCodec =  AV_CODEC_ID_PCM_ALAW;
	AVCodec *output_codec = avcodec_find_encoder(idCodec);
	RETERR_ON_FALSE(output_codec, AVERROR_EXIT, 
		boost::format("Could not find an encoder for codec_id=%1%.\n") % idCodec);


	auto strmOut = avformat_new_stream(ctxFormat_, output_codec);
	strmOut->time_base = { 1, 8000 };
	AVCodecContext *codecctxOUT = strmOut->codec;


	/**
	* Set the basic encoder parameters.
	* The input file's sample rate is used to avoid a sample rate conversion.
	*/
	codecctxOUT->channels = 1;
	codecctxOUT->channel_layout = av_get_default_channel_layout(codecctxOUT->channels);
	codecctxOUT->sample_fmt =  output_codec->sample_fmts[0];
	codecctxOUT->sample_rate	= RTP_OUT_RATE;
	codecctxOUT->bit_rate		= RTP_OUT_RATE;
	codecctxOUT->time_base		= { 1, codecctxOUT->sample_rate };

	RETURN_AVERROR(avcodec_open2(strmOut->codec, output_codec, nullptr),
		"Could not open output codec");

	/*char buffSDP[2048] = { 0 };
	av_sdp_create(&ctxFormat_, 1, buffSDP, sizeof(buffSDP)-1);
	cou << "-------------ffmpeg SDP-------------\n" << buffSDP <<
		"\n-----------------------------\n";*/

	return m_lastError;
}
//-----------------------------------------------------------------------
int CDestFusion::openFile(const char *filename)
{
	_cleanup();
	isRTP_ = false;
	BOOST_SCOPE_EXIT_ALL(&)
	{
		if (this->m_lastError < 0)
			this->_cleanup();
	};
	AVIOContext *output_io_context = nullptr;
	RETURN_AVERROR(
		avio_open(&output_io_context, filename, AVIO_FLAG_WRITE),
		boost::format("Could not open output file '%s'") % filename
		);

	/** Create a new format context for the output container format. */
	if (!(ctxFormat_ = avformat_alloc_context()))
		cerr << "Could not allocate output format contex";

	/** Associate the output file (pointer) with the container format context. */
	ctxFormat_->pb = output_io_context;

	/** Guess the desired container format based on the file extension. */
	ctxFormat_->oformat = av_guess_format(nullptr, filename, nullptr);
	RETERR_ON_FALSE(ctxFormat_->oformat, AVERROR_EXIT, "Could not find output file format");

	av_strlcpy(ctxFormat_->filename, filename,
		sizeof(ctxFormat_->filename));

	/** Find the encoder to be used by its name. */
	AVCodec *encoder = avcodec_find_encoder(AV_CODEC_ID_PCM_S16LE);
	RETERR_ON_FALSE(encoder, AVERROR_EXIT, "Could not find an PCM encoder");

	AVStream *stream = avformat_new_stream(ctxFormat_, encoder);
	/** Create a new audio stream in the output file container. */
	RETERR_ON_FALSE(stream, AVERROR(ENOMEM), "Could not create new stream");
	
	/** Save the encoder context for easiert access later. */
	AVCodecContext* ctxCodecOut = stream->codec;

	/**
	* Set the basic encoder parameters.
	*/
	ctxCodecOut->channels = 1;
	ctxCodecOut->channel_layout = av_get_default_channel_layout(ctxCodecOut->channels);
	ctxCodecOut->sample_rate = 8000;
	ctxCodecOut->sample_fmt = AV_SAMPLE_FMT_S16;
	//ctxCodecOut->bit_rate = 8000;
	//(*output_codec_context)->bit_rate       = input_codec_context->bit_rate;


	/**
	* Some container formats (like MP4) require global headers to be present
	* Mark the encoder so that it behaves accordingly.
	*/
	if (ctxFormat_->oformat->flags & AVFMT_GLOBALHEADER)
		ctxCodecOut->flags |= CODEC_FLAG_GLOBAL_HEADER;

	/** Open the encoder for the audio stream to use it later. */
	RETURN_AVERROR(
		avcodec_open2(ctxCodecOut, encoder, nullptr),
		"Could not open output codec"
		);

	return m_lastError;
}
//-----------------------------------------------------------------------
void CDestFusion::_finalizeRTPThread()
{
	if (pRTPThread_)
	{
		//printf("\n_finalizeRTPThread()\n");
		buffRTP_.terminate();
		pRTPThread_->join();
		delete pRTPThread_;
		pRTPThread_ = nullptr;
	}
}
//-----------------------------------------------------------------------
int CDestFusion::runBegin()
{
	if (!isValid())
		return LastError();
	_reinitFilters();

	RETURN_AVERROR(
		avformat_write_header(ctxFormat_, NULL),
		"Could not write output file header"
		);

	if (isValid())
	{
		_finalizeRTPThread();
		//cou << "\nCall from runBegin _finalizeRTPThread(); done";
		if (isRTP_)
			pRTPThread_ = new std::thread (&CDestFusion::_threadRTPfunction, this);
	}
	//cout << "\n    1 Exit from runBegin";
	return LastError();
}
//-----------------------------------------------------------------------
int CDestFusion::runEnd()
{
	assert(ctxFormat_);
	//cout << "\n    _finalizeRTPThread();";
	_finalizeRTPThread();
	//cou << "\ncall from runEnd: _finalizeRTPThread(); done";

	RETURN_AVERROR(
		av_write_trailer(ctxFormat_),
		"Could not write output file trailer"
		);
	//cout << "\n    exit from runEnd;";
	return LastError();
}
//-----------------------------------------------------------------------
int CDestFusion::run()
{
/*
	assert(ctxFormat_);
	_reinitFilters();

	if (LastError() < 0)
		return LastError();

	RETURN_AVERROR(
		avformat_write_header(ctxFormat_, NULL),
		"Could not write output file header"
		);

	if (isRTP_)
	{
		std::thread thRTP(&CDestFusion::_threadRTPfunction, this);
		_proceedIO();
		thRTP.join();
	}
	else
	{		
		_proceedIO();
	}*/
	//cout << "\n   1 runBegin();";
	runBegin();
	if (isValid())
	{
		//cout << "\n   1 _proceedIO();";
		_proceedIO();
		//cout << "\n   1 runEnd();";
		runEnd();
	}
	//cout << "\n   1 Exit from run";
	return LastError();
}
//-----------------------------------------------------------------------
void CDestFusion::_cleanup()
{
	_finalizeRTPThread();
	if (ctxFormat_)
	{
		avio_close(ctxFormat_->pb);
		avformat_free_context(ctxFormat_);
		m_lastError = 0;
	}
}
//-----------------------------------------------------------------------
void CDestFusion::_dumpGraph()
{
	//cout << "============ dumpGraph ======================\n";
	char* dump = avfilter_graph_dump(filtGraf_, nullptr);
	if (dump)
	{
		//test222
		av_log(NULL, AV_LOG_ERROR, "Graph :\n%s\n", dump);
		av_free(dump);
	}
}
//-----------------------------------------------------------------------
/** Encode one frame worth of audio to the output file. */
int CDestFusion::_encode_audio_frame(AVFrame *frame, int *data_present)
{
	/** Packet used for temporary storage. */
	SHP_CAVPacket shPacket = std::make_shared<CAVPacket>();
	
	/**
	* Encode the audio frame and store it in the temporary packet.
	* The output audio stream encoder is used to do this.
	*/
	RETURN_AVERROR(
		avcodec_encode_audio2(CodecCTX(), shPacket.get(),
			frame, data_present),
		"Could not encode frame"
		);

	/** Write one audio frame from the temporary packet to the output file. */
	if (*data_present) 
	{
		if (isRTP_)
			buffRTP_.push(shPacket);
		else
			RETURN_AVERROR(
			av_write_frame(ctxFormat_, shPacket.get()),
				"Could not write frame");
	}

	return 0;
}
//-----------------------------------------------------------------------
int CDestFusion::_initGraph()
{
	assert(!cllSrcRefs_.empty());

	if (ctxMix_)
	{
		for (size_t i = 0; i < cllSrcRefs_.size(); ++i)
		{
			RETURN_AVERROR(
				avfilter_link(cllSrcRefs_[i].pctxFilter, 0, ctxMix_, i),
				"Error connecting filters");
		}

		RETURN_AVERROR(
			avfilter_link(ctxMix_, 0, ctxSink_, 0),
			"Linking Sink"
			);
	}
	else
	{
		RETURN_AVERROR(
			avfilter_link(cllSrcRefs_[0].pctxFilter, 0, ctxSink_, 0),
			"Linking Sink"
			);
	}


	/* Configure the graph. */
	RETURN_AVERROR(
		avfilter_graph_config(filtGraf_, NULL),
		"Configuring graph"
		);

	//_dumpGraph();//медиа печать

	return m_lastError;
}
//-----------------------------------------------------------------------
int CDestFusion::_initMixFilter()
{
	assert(!ctxMix_);

	if (cllSrcRefs_.size() > 1)
	{
		/****** amix ******* */
		/* Create mix filter. */

		RETERR_ON_FALSE(gl_amix, AVERROR_FILTER_NOT_FOUND, "Could not find the mix filter");

		RETURN_AVERROR(
			avfilter_graph_create_filter(&ctxMix_, gl_amix, "CDestCtx amix",
			str(boost::format("inputs=%1%") % cllSrcRefs_.size()).c_str(),
			NULL, filtGraf_),
			"Cannot create audio amix filter");
	}


	return m_lastError;
}
//-----------------------------------------------------------------------
int CDestFusion::_initSink()
{
	if (m_lastError)
		return m_lastError;

	assert(!ctxSink_);

	/* Finally create the abuffersink filter;
	* it will be used to get the filtered data out of the graph. */
	RETERR_ON_FALSE(gl_abuffersink, AVERROR_FILTER_NOT_FOUND,
		"Could not find the abuffersink filter"
		);

	ctxSink_ = avfilter_graph_alloc_filter(filtGraf_, gl_abuffersink, "CDestCtx sink");
	RETERR_ON_FALSE(ctxSink_, AVERROR(ENOMEM), "Could not allocate the abuffersink instance.");

	/* Same sample fmts as the output file. */
	static const enum AVSampleFormat
		Fmts[] = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };

	RETURN_AVERROR(
		av_opt_set_int_list(ctxSink_, "sample_fmts",
		Fmts, AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN),
		"Could set options to the abuffersink instance");

	static const int64_t out_channel_layouts[] = { AV_CH_LAYOUT_MONO, -1 };
	RETURN_AVERROR(
		av_opt_set_int_list(ctxSink_,
		"channel_layouts", out_channel_layouts, -1, AV_OPT_SEARCH_CHILDREN),
		"Cannot set output channel layout");

	static const int out_sample_rates[] = { 8000, -1 };
	RETURN_AVERROR(
		av_opt_set_int_list(ctxSink_,
		"sample_rates", out_sample_rates, -1, AV_OPT_SEARCH_CHILDREN),
		"Cannot set output sample rate");
/*
	char ch_layout[64];
	av_get_channel_layout_string(ch_layout, sizeof(ch_layout), 0, CodecCTX()->channels);

	av_opt_set(ctxSink_, "channel_layout", ch_layout, AV_OPT_SEARCH_CHILDREN);
*/
	/*
	RETURN_AVERROR(
	av_opt_set(ctxSink_, "channel_layout", ch_layout, AV_OPT_SEARCH_CHILDREN),
	"Could set options to the abuffersink instance"
	);
	*/

	RETURN_AVERROR(
		avfilter_init_str(ctxSink_, NULL),
		"Could not initialize the abuffersink instance");


	return m_lastError;
}
//-----------------------------------------------------------------------
int CDestFusion::proceedIO_step()
{
	if (!isActive())
		return AVERROR_EXIT;
	bool data_present_in_graph = false;

	/************************************************************************
	Read and push to FilterGraph
	************************************************************************/
	for (auto& srcRef : cllSrcRefs_)
	{
		if (srcRef.finished || !srcRef.to_read){
			continue;}
		srcRef.to_read = false;
		bool bEOF = false;
		//cout << "\n     1 IN STEP BEFORE GETNEXTDECODED";
		/** Decode one frame worth of audio samples. */
		SHP_CScopedPFrame shpFrame = srcRef.shpSrc->getNextDecoded(bEOF);
		//cout << "\n     1 IN STEP AFTER GETNEXTDECODED";
		const AVFrame * const frame = shpFrame ? shpFrame->frame : nullptr;


		/**
		* If we are at the end of the file and there are no more samples
		* in the decoder which are delayed, we are actually finished.
		* This must not be treated as an error.
		*/
		//cout<<"\nHI I`M IN STEP 2\n";
		if (bEOF && !frame) {
			srcRef.finished = true;
			av_log(NULL, AV_LOG_INFO, "Input n#%s finished. Write NULL frame \n", srcRef.name());

			RETURN_AVERROR(
				av_buffersrc_write_frame(srcRef.pctxFilter, nullptr),
				"Writing EOF null frame for input %d\n"
				);
		}
		else if (frame)
		{ /** If there is decoded data, convert and store it
			push the audio data from decoded frame into the filtergraph */
			//printf("\n test333 \n");
			RETURN_AVERROR(
				av_buffersrc_write_frame(srcRef.pctxFilter, frame),
				"Feeding the audio filtergraph\n"
				);


			auto codecctx = srcRef.shpSrc->CodecCTX();
			//#ifdef _DEBUG
/*
			av_log(NULL, AV_LOG_INFO, "add %d samples on input %s (%d Hz, time=%f, ttime=%f)\n",
				frame->nb_samples, srcRef.name(), codecctx->sample_rate,
				(double)frame->nb_samples / codecctx->sample_rate,
				(double)(srcRef.total_samples += frame->nb_samples) / codecctx->sample_rate);
*/
			//#endif // _DEBUG

		}

		data_present_in_graph = frame || data_present_in_graph;
	}
	//cout<<"\nHI I`M IN STEP 3\n";
	/************************************************************************
	Pull from FilterGraph and encode
	************************************************************************/
	//cout<<"\nHI I`M IN STEP TOO\n";
	if (data_present_in_graph)
	{
		RETURN_AVERROR(
			_pullAndEncode(),
			"CDestFusion::_pullAndEncode");
	}
	else
	{
		av_log(NULL, AV_LOG_INFO, "No data in graph\n");
		for (auto& srcRef : cllSrcRefs_)
			srcRef.to_read = true;
	}
	//printf("\n test222\n");

	//cout << "\n     1 Exit from proceed io step";
	return LastError();
}
//-----------------------------------------------------------------------
int CDestFusion::_proceedIO()
{
	//cout << "\n    1 Call from _proceedIO()";
	do
	{
		//cout << "PIOS-";
		proceedIO_step();
		//cout << "PIOSend ";
	} 
	while (isValid() && isActive() && ExistsNotFinishedSrc() );
	//cout << "\n    1 Exit from _proceedIO()";
	return LastError();
}
//-----------------------------------------------------------------------
int CDestFusion::_pullAndEncode()
{
	CScopedPFrame filt_frame;
	RETERR_ON_FALSE(filt_frame.isValid(), AVERROR(ENOMEM), "Frame is invalid");

	/* pull filtered audio from the filtergraph */
	while (1) {
		int ret = av_buffersink_get_frame(ctxSink_, filt_frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		{
			for (TSrcRef& srcRef : cllSrcRefs_)
			{
				auto nbFailed = av_buffersrc_get_nb_failed_requests(srcRef.pctxFilter);
				if (nbFailed > 0)
				{
					srcRef.to_read = true;
#ifdef _DEBUG
/*
					av_log(NULL, AV_LOG_INFO,
						"%s is needed to read input (%d requests)\n",
						srcRef.name(), nbFailed);*/
#endif // _DEBUG
				}
			}
			break;
		}
		RETURN_AVERROR(ret, "pull filtered audio from");

/*
		av_log(NULL, AV_LOG_INFO, "remove %d samples from sink (%d Hz, time=%f, ttime=%f)\n",
			filt_frame->nb_samples, CodecCTX()->sample_rate,
			(double)filt_frame->nb_samples / CodecCTX()->sample_rate,
			(double)(total_out_samples_ += filt_frame->nb_samples) / CodecCTX()->sample_rate);
*/

		static int64_t pts_ = 0;
		filt_frame->pts = pts_;
		pts_ += filt_frame->nb_samples;

		int gotted = 0;
		RETURN_AVERROR(
			_encode_audio_frame(filt_frame, &gotted),
			"encode_audio_frame");

		av_frame_unref(filt_frame);
	}

	return m_lastError;
}
//-----------------------------------------------------------------------
void CDestFusion::_reinitFilters()
{
	//printf("\n_initMixFilter();\n");
	_initMixFilter();
	//printf("\n__initSink();\n");
	_initSink();
	//test222
	//printf("\n__initGraph();\n");
	_initGraph();
}
//-----------------------------------------------------------------------
void CDestFusion::_threadRTPfunction()
{
/*
	assert(!m_sdpOUT.cllMedia.empty());
	const auto& m = m_sdpOUT.cllMedia.front();
	const int maxchunk = m.maxptime > 0 ? (m.maxptime * 8000) / 1000 : 160;
	const int stdchunk = m.ptime > 0
		? (m.ptime * 8000) / 1000
		: std::min(maxchunk, 160);
*/

	const int szChunk = ptimeRTP_ > 0
		? (ptimeRTP_ * RTP_OUT_RATE) / 1000
		: 160;

	int szSrc = 0;
	SHP_CAVPacket packSrc;
	decltype(AVPacket::data) pSrc	= nullptr;
	decltype(AVPacket::pts)  pts	= 0;
	auto tpNext = time_point::clock::now();

	buffRTP_.resume();
	while (isActive())
	{
		CAVPacket pktRTP(szChunk);
		if (!pktRTP)
			return; // С памятью проблема

		int szRTP = std::min(szChunk, szSrc);
		if (pSrc)//pDataSrc == nullptr при первом проходе
			memcpy(pktRTP.data, pSrc, szRTP); //при первом проходе ничего не копируем
		bool bPackChanged = false;
		while (pktRTP.size > szRTP)
		{
			//Доукомплектовываем данными
			packSrc = buffRTP_.pop();

			bPackChanged = true;
			if (!packSrc)
				break; //Данных больше нет

			const auto szAdd = std::min(packSrc->size, pktRTP.size - szRTP);
			memcpy(pktRTP.data + szRTP, packSrc->data, szAdd);
			pSrc = packSrc->data + szAdd;
			szSrc = packSrc->size - szAdd;
			szRTP += szAdd;
			assert(pktRTP.size >= szRTP);
		}
		if (!bPackChanged)
		{
			pSrc += szRTP;
			szSrc -= szRTP;
			assert(packSrc && szSrc >= 0);
		}

		if (pktRTP.size > szRTP)
		{
			pktRTP.shrink_to(szRTP); //остатки данных 
			assert(!packSrc); //Больше данных нет - последняя отправка
		}

		pktRTP.pts = pktRTP.dts = pts;
		pts += pktRTP.size;
		if(_writeRTPpacket(tpNext, pktRTP) < 0)
			buffRTP_.terminate();

		if (!packSrc)//TODO: нах?
			break;
	}
}
//-----------------------------------------------------------------------
int CDestFusion::_writeRTPpacket(time_point& tp, CAVPacket& pktRTP)
{
	std::this_thread::sleep_until(tp);
	//timestamp следующего пакета
	tp += /*CRTPMedia::time_point::clock::now() + */
		chrono::microseconds((long long)(10000.*pktRTP.size / 80.));
//	cou << boost::format("%1% rtpthread send %2% bytes\n") % std::this_thread::get_id() % pktRTP.size;
	return av_interleaved_write_frame(ctxFormat_, &pktRTP);
}
//-----------------------------------------------------------------------
bool CDestFusion::ExistsNotFinishedSrc() const
{
	return cllSrcRefs_.cend() !=
		std::find_if(cllSrcRefs_.cbegin(), cllSrcRefs_.cend(),
			[](const TSrcRef& src){return !src.finished; }
		);
}
