#pragma once
#include "stdafx.h"
#include "CAnnRQNT.h"

#define snprintf _snprintf
#define VOLUME_VAL 0.90

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CAnnRQNT::init()
{
	av_register_all();
	avcodec_register_all();
	avfilter_register_all();
	avformat_network_init();

	if (open_input_file() < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Error while opening file 1\n");
		system("pause");
	}
	if (open_output_file() < 0)
	{
		cout << "\nerror open output";
		system("pause");
	}
	process_all();

}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
static char *const get_error_text(const int error)
{
	static char error_buffer[255];
	av_strerror(error, error_buffer, sizeof(error_buffer));
	return error_buffer;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
/** Open an input file and the required decoder. */
int CAnnRQNT::open_input_file()
{
	AVCodec *input_codec;
	int error;

	/** Open the input file to read from it. */
	if ((error = avformat_open_input(&ifcx, filename.c_str(), NULL,
		NULL)) < 0) 
	{
		av_log(NULL, AV_LOG_ERROR, "Could not open input file '%s' (error '%s')\n", filename, get_error_text(error));
		ifcx = NULL;
		return error;
	}

	/** Get information on the input file (number of streams etc.). */
	if ((error = avformat_find_stream_info(ifcx, NULL)) < 0) 
	{
		av_log(NULL, AV_LOG_ERROR, "Could not open find stream info (error '%s')\n", get_error_text(error));
		avformat_close_input(&ifcx);
		return error;
	}

	/** Make sure that there is only one stream in the input file. */
	if ((ifcx)->nb_streams != 1) 
	{
		av_log(NULL, AV_LOG_ERROR, "Expected one audio input stream, but found %d\n", (ifcx)->nb_streams);
		avformat_close_input(&ifcx);
		return AVERROR_EXIT;
	}

	/** Find a decoder for the audio stream. */
	if (!(input_codec = avcodec_find_decoder((ifcx)->streams[0]->codec->codec_id))) 
	{
		av_log(NULL, AV_LOG_ERROR, "Could not find input codec\n");
		avformat_close_input(&ifcx);
		return AVERROR_EXIT;
	}

	/** Open the decoder for the audio stream to use it later. */
	if ((error = avcodec_open2((ifcx)->streams[0]->codec, input_codec, NULL)) < 0) 
	{
		av_log(NULL, AV_LOG_ERROR, "Could not open input codec (error '%s')\n", get_error_text(error));
		avformat_close_input(&ifcx);
		return error;
	}
	/** Save the decoder context for easier access later. */
	iccx = (ifcx)->streams[0]->codec;

	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
/**
* Open an output file and the required encoder.
* Also set some basic encoder parameters.
* Some of these parameters are based on the input file's parameters.
*/
int CAnnRQNT::open_output_file()
{
	AVIOContext *output_io_context = NULL;
	AVStream *stream = NULL;
	AVCodec *output_codec = NULL;
	int error;

	/** Open the output file to write to it. */
	if ((error = avio_open(&output_io_context, filename.c_str(), AVIO_FLAG_WRITE)) < 0) 
	{
		av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s' (error '%s')\n", filename, get_error_text(error));
		return error;
	}

	/** Create a new format context for the output container format. */
	if (!(out_ifcx = avformat_alloc_context())) 
	{
		av_log(NULL, AV_LOG_ERROR, "Could not allocate output format context\n");
		return AVERROR(ENOMEM);
	}

	/** Associate the output file (pointer) with the container format context. */
	(out_ifcx)->pb = output_io_context;

	/** Guess the desired container format based on the file extension. */
	if (!((out_ifcx)->oformat = av_guess_format(NULL, filename.c_str(), NULL))) 
	{
		av_log(NULL, AV_LOG_ERROR, "Could not find output file format\n");
		goto cleanup;
	}

	av_strlcpy((out_ifcx)->filename, filename.c_str(),
		sizeof((out_ifcx)->filename));

	/** Find the encoder to be used by its name. */
	if (!(output_codec = avcodec_find_encoder(AV_CODEC_ID_PCM_S16LE))) 
	{
		av_log(NULL, AV_LOG_ERROR, "Could not find an PCM encoder.\n");
		goto cleanup;
	}

	/** Create a new audio stream in the output file container. */
	if (!(stream = avformat_new_stream(out_ifcx, output_codec))) 
	{
		av_log(NULL, AV_LOG_ERROR, "Could not create new stream\n");
		error = AVERROR(ENOMEM);
		goto cleanup;
	}

	/** Save the encoder context for easiert access later. */
	out_iccx = stream->codec;

	/**
	* Set the basic encoder parameters.
	*/
	(out_iccx)->channels = 1;
	(out_iccx)->channel_layout = av_get_default_channel_layout(1);
	(out_iccx)->sample_rate = iccx->sample_rate;
	(out_iccx)->sample_fmt = AV_SAMPLE_FMT_S16;

	av_log(NULL, AV_LOG_INFO, "output bitrate %d\n", (out_iccx)->bit_rate);

	/**
	* Some container formats (like MP4) require global headers to be present
	* Mark the encoder so that it behaves accordingly.
	*/
	if ((out_ifcx)->oformat->flags & AVFMT_GLOBALHEADER)
		(out_iccx)->flags |= CODEC_FLAG_GLOBAL_HEADER;

	/** Open the encoder for the audio stream to use it later. */
	if ((error = avcodec_open2(out_iccx, output_codec, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Could not open output codec (error '%s')\n", get_error_text(error));
		goto cleanup;
	}

	return 0;

cleanup:
	avio_close((out_ifcx)->pb);
	avformat_free_context(out_ifcx);
	out_ifcx = NULL;
	return error < 0 ? error : AVERROR_EXIT;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
/** Initialize one audio frame for reading from the input file */
int CAnnRQNT::init_input_frame(AVFrame **frame)
{
	if (!(*frame = av_frame_alloc())) 
	{
		av_log(NULL, AV_LOG_ERROR, "Could not allocate input frame\n");
		return AVERROR(ENOMEM);
	}
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
/** Initialize one data packet for reading or writing. */
void CAnnRQNT::init_packet(AVPacket *packet)
{
	av_init_packet(packet);
	/** Set the packet data and size so that it is recognized as being empty. */
	packet->data = NULL;
	packet->size = 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
/** Decode one audio frame from the input file. */
int CAnnRQNT::decode_audio_frame(AVFrame *frame, int *data_present)
{
	/** Packet used for temporary storage. */
	AVPacket input_packet;
	int error;
	init_packet(&input_packet);

	/** Read one audio frame from the input file into a temporary packet. */
	if ((error = av_read_frame(ifcx, &input_packet)) < 0) {
		/** If we are the the end of the file, flush the decoder below. */
		if (error == AVERROR_EOF)
			return -100;
		else {
			av_log(NULL, AV_LOG_ERROR, "Could not read frame (error '%s')\n",
				get_error_text(error));
			return error;
		}
	}

	/**
	* Decode the audio frame stored in the temporary packet.
	* The input audio stream decoder is used to do this.
	* If we are at the end of the file, pass an empty packet to the decoder
	* to flush it.
	*/
	if ((error = avcodec_decode_audio4(iccx, frame,
		data_present, &input_packet)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Could not decode frame (error '%s')\n",
			get_error_text(error));
		av_free_packet(&input_packet);
		return error;
	}

	/**
	* If the decoder has not been flushed completely, we are not finished,
	* so that this function has to be called again.
	*/
	av_free_packet(&input_packet);
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
/** Encode one frame worth of audio to the output file. */
int CAnnRQNT::encode_audio_frame(AVFrame *frame, int *data_present)
{
	/** Packet used for temporary storage. */
	AVPacket output_packet;
	int error;
	init_packet(&output_packet);

	/**
	* Encode the audio frame and store it in the temporary packet.
	* The output audio stream encoder is used to do this.
	*/
	if ((error = avcodec_encode_audio2(out_iccx, &output_packet, frame, data_present)) < 0) 
	{
		av_log(NULL, AV_LOG_ERROR, "Could not encode frame (error '%s')\n", get_error_text(error));
		av_free_packet(&output_packet);
		return error;
	}

	/** Write one audio frame from the temporary packet to the output file. */
	/*if (*data_present) {
		if ((error = av_write_frame(out_ifcx, &output_packet)) < 0) {
			av_log(NULL, AV_LOG_ERROR, "Could not write frame (error '%s')\n",
				get_error_text(error));
			av_free_packet(&output_packet);
			return error;
		}

		av_free_packet(&output_packet);
	}*/

	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------