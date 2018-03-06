#include "stdafx.h"
#include "SrcCommon.h"


/************************************************************************
	CSrcCommon	
************************************************************************/
int CSrcCommon::decode_audio_frame(AVFrame *frame,
	int *gotted, bool *finished)
{
	/** Packet used for temporary storage. */
	CAVPacket input_packet;
	int error = 0;

	/** Read one audio frame from the input file into a temporary packet. */
	if ((error = av_read_frame(ctxFormat_, &input_packet)) < 0) {
		/** If we are the the end of the file, flush the decoder below. */
		*finished = true;

		/*
		if (error == AVERROR_EOF)
		*finished = 1;
		else {
		av_log(NULL, AV_LOG_ERROR, "Could not read frame (error '%s')\n",
		get_error_pchar(error));
		return error;
		}
		*/
	}

	/**
	* Decode the audio frame stored in the temporary packet.
	* The input audio stream decoder is used to do this.
	* If we are at the end of the file, pass an empty packet to the decoder
	* to flush it.
	*/
	RETURN_AVERROR(avcodec_decode_audio4(_CodecCTX(), frame, gotted, &input_packet),
		"Could not decode frame"
		);

	/**
	* If the decoder has not been flushed completely, we are not finished,
	* so that this function has to be called again.
	*/
	if (*finished && *gotted)
		*finished = false;


	return 0;
}

//-----------------------------------------------------------------------
SHP_CScopedPFrame CSrcCommon::getNextDecoded(bool& bEOF)
{
	SHP_CScopedPFrame shResult;

	/** Packet used for temporary storage. */
	CAVPacket packIN;
	int error = av_read_frame(ctxFormat_, &packIN);

	/** Read one audio frame from the input file into a temporary packet. */
	if (error < 0) {
		m_lastError = error;
		/** If we are the the end of the file, flush the decoder below. */
		bEOF = true;
		return shResult;
		/*
		if (error == AVERROR_EOF)
		*finished = 1;
		else {
		av_log(NULL, AV_LOG_ERROR, "Could not read frame (error '%s')\n",
		get_error_pchar(error));
		return error;
		}
		*/
	}

	/**
	* Decode the audio frame stored in the temporary packet.
	* The input audio stream decoder is used to do this.
	* If we are at the end of the file, pass an empty packet to the decoder
	* to flush it.
	*/
	int gotted = 0;
	shResult = std::make_shared<CScopedPFrame>();

	m_lastError = avcodec_decode_audio4(_CodecCTX(), *shResult, &gotted, &packIN);
	if (m_lastError < 0)
	{
		cerr << "Could not decode frame";
		return SHP_CScopedPFrame();
	}


	/**
	* If the decoder has not been flushed completely, we are not finished,
	* so that this function has to be called again.
	*/
	if (bEOF && gotted)
		bEOF = false;

	return shResult;
}

//-----------------------------------------------------------------------
int CSrcCommon::open(const string& strFile)
{
	_closeFormat();
	//whatfor?
	//strName_ = str(boost::format("%1%_%2%") % this % strFile);


	m_lastError = 0;
	const char* filename = strFile.c_str();

	BOOST_SCOPE_EXIT_ALL(&)
	{
		if (m_lastError < 0)
			_closeFormat();
	};

	/** Open the input file to read from it. */
	RETURN_AVERROR(
		avformat_open_input(&ctxFormat_, filename, NULL, NULL),
		boost::format("Could not open input file %1%") % filename
		);

	/** Get information on the input file (number of streams etc.). */
	RETURN_AVERROR(
		avformat_find_stream_info(ctxFormat_, NULL),
		"Could not open find stream info"
		);

	//Выбор индекса потока
	for (unsigned i = 0; idxStream_ == -1 && i < ctxFormat_->nb_streams; ++i)
		idxStream_ = ctxFormat_->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO
		? i : -1;

	/** Find a decoder for the audio stream. */
	AVCodec *input_codec = avcodec_find_decoder(_CodecCTX()->codec_id);
	if (!input_codec)
		RETURN_AVERROR(AVERROR_EXIT, "Could not find input codec");

	/** Open the decoder for the audio stream to use it later. */
	RETURN_AVERROR(
		avcodec_open2(_CodecCTX(), input_codec, nullptr),
		"Could not open input codec"
		);

	if (!_CodecCTX()->channel_layout)
		_CodecCTX()->channel_layout = av_get_default_channel_layout(_CodecCTX()->channels);

	//информация о медиа
	av_dump_format(ctxFormat_, 0, filename, 0);

	return m_lastError;
}

//-----------------------------------------------------------------------
void CSrcCommon::_closeFormat()
{
	avformat_close_input(&ctxFormat_);
}
