#pragma once
#include "stdafx.h"
#include "CRTPReceive.h"
#include <boost/thread.hpp>
using namespace std;


void CRTPReceive::loggit(string a)
{
	fprintf(FileLogMixer, ("\n" + a + "\n//-------------------------------------------------------------------").c_str());
	fflush(FileLogMixer);
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
	loggit("int CRTPReceive::decode_audio_frame");
	/** Packet used for temporary storage. */
	AVPacket input_packet;
	int error;
	init_packet(&input_packet);
	SHP_CAVPacket2 shpPacket;

	int szPack = vecSock[i]->receive_from(boost::asio::buffer(data, 8100), vecEndpoint[i]);
	if (szPack > 12)
	{
		rtp[i].reset(new CAVPacket2(12));
		shpPacket.reset(new CAVPacket2(szPack));
		memcpy(rtp[i]->data, data, 12);
		memcpy(shpPacket->data, data + 12, szPack - 12);
	}
	else
	{
		shpPacket.reset(new CAVPacket2(0));
	}
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
	if ((error = avcodec_decode_audio4(ext.iccx[i], frame, data_present, shpPacket.get())) < 0)
	{
		string s(get_error_text(error));
		loggit("Could not decode frame (error" + s);
		av_log(NULL, AV_LOG_ERROR, "Could not decode frame (error '%s')\n",
			get_error_text(error));
		av_free_packet(&input_packet);
		return error;
	}
	if (*finished && *data_present)
		*finished = 0;

	av_free_packet(&input_packet);
	loggit("int CRTPReceive::decode_audio_frame END");
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
	if ((error = avcodec_encode_audio2(ext.out_iccx[i], &output_packet,
		frame, data_present)) < 0) 
	{
		string s(get_error_text(error));
		loggit("Could not encode frame (error " + s);
		av_log(NULL, AV_LOG_ERROR, "Could not encode frame (error '%s')\n", get_error_text(error));
		av_free_packet(&output_packet);
		return error;
	}

	/** Write one audio frame from the temporary packet to the output file. */
	if (*data_present) 
	{	
		try
		{
			send.reset(new CAVPacket2(output_packet.size + 12));
			memcpy(send->data, rtp[i]->data, 12);
			memcpy(send->data + 12, output_packet.data, output_packet.size);
			loggit("sending " + to_string(send->size - 12) + "bytes to ip " + vecEndpoint[i].address().to_string() + " and port=" + to_string(vecEndpoint[i].port()));
			vecSock[i]->send_to(boost::asio::buffer(send->data, send->size - 12), vecEndpoint[i]);
			av_free_packet(&output_packet);
		}
		catch (std::exception& e)
		{
				loggit("Exception:" + to_string(*e.what()));
				cerr << "Exception: " << e.what() << "\n";
				return -1;
			
		}
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
	while ((nb_finished < tracks) && (on==true))
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
						ret = av_buffersrc_write_frame(ext.afcx[j].src[i - 1], NULL);
					else
						ret = av_buffersrc_write_frame(ext.afcx[j].src[i], NULL);
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
					//loggit("i=" + to_string(i) + " j= " + to_string(j));
					if (i == j) continue;

					if (j < i)
						ret = av_buffersrc_write_frame(ext.afcx[j].src[i - 1], frame);
					else
						ret = av_buffersrc_write_frame(ext.afcx[j].src[i], frame);
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
					ret = av_buffersink_get_frame(ext.sinkVec[k], filt_frame);
					if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
					{
						for (int i = 0; i < tracks - 1; i++)
						{
							if (av_buffersrc_get_nb_failed_requests(ext.afcx[k].src[i]) > 0)
							{
								input_to_read[i] = 1;
								loggit("Need to read input " + to_string(i));
							}
						}
						break;
					}
					if (ret < 0){ std::cout << "\n 2.5"; goto end; }
					loggit("remove samples from sink ( Hz, time=, ttime=)\n");
					//if (ret = encode_audio_frame_file(filt_frame, &data_present, k)< 0)
					if (ret = encode_audio_frame(filt_frame, &data_present, k)< 0)
					{ 
						loggit("if (ret = encode_audio_frame(filt_frame, out_ifcx[k], out_iccx[k], &data_present)< 0) FAILED");
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
	ready = true;
	loggit("Process all END");
	return 0;

end:

	if (ret < 0 && ret != AVERROR_EOF) 
	{
		loggit("av_log(NULL, AV_LOG_ERROR, Error occurred : \n, av_err2str(ret));");
		printf("av_log(NULL, AV_LOG_ERROR, Error occurred : \n, av_err2str(ret));");
		system("pause");
		std::exit(1);
	}
	system("pause");
	std::exit(2);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::add_track(vector<string> input_SDPs, vector<string> IPs, vector<int> my_ports, vector<int> remote_ports)
{
	on = false;
	Init.reset(new CMixInit(input_SDPs, IPs, my_ports, remote_ports));
	
	while (ready == false){ cout << "f"; }
	++tracks;
	ext = Init->data;

	ready = false;
	reinit_sockets(my_ports);
	on = true;
	process_all();
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CRTPReceive::encode_audio_frame_file(AVFrame *frame, int *data_present, int i)
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
	if ((error = avcodec_encode_audio2(ext.out_iccx[i], &output_packet,
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

	if (*data_present)
	{
		loggit("encode for i=" + to_string(i));
		if ((error = av_interleaved_write_frame(ext.out_ifcx[i], &output_packet)) < 0)
		{
		string s(get_error_text(error));
		loggit("Could not write frame (error " + s);
		av_log(NULL, AV_LOG_ERROR, "Could not write frame (error '%s')\n",
		get_error_text(error));
		av_free_packet(&output_packet);
		return error;
		}
		av_free_packet(&output_packet);
	}

	loggit("int CRTPReceive::encode_audio_frame END");
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::init_sockets(vector<int> my_ports)
{
	for (int i = 0; i < tracks; ++i)
	{
		SHP_Socket a;
		udp::endpoint sender_endpoint;
		a.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), my_ports[i])));
		a->receive_from(boost::asio::buffer(data, 8000), sender_endpoint);

		vecEndpoint.push_back(sender_endpoint);
		vecSock.push_back(a);
		SHP_CAVPacket2 c;
		rtp.push_back(c);
	}
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::reinit_sockets(vector<int> my_ports)
{
	for (unsigned int i = 0; i < vecSock.size(); ++i)
	{
		vecSock[i]->close();
	}
	vecSock.clear();
	rtp.clear();
	vecEndpoint.clear();
	for (int i = 0; i < tracks; ++i)
	{
		SHP_Socket a;
		udp::endpoint sender_endpoint;
		a.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), my_ports[i])));
		a->receive_from(boost::asio::buffer(data, 8000), sender_endpoint);

		vecEndpoint.push_back(sender_endpoint);
		vecSock.push_back(a);
		SHP_CAVPacket2 c;
		rtp.push_back(c);
	}
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

