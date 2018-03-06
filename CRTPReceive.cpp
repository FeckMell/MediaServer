#pragma once
#include "stdafx.h"
#include "CRTPReceive.h"
#define _DEBUG_time
using namespace std;

void StartReceive(CRTPReceive* a, int i)
{
	a->receive(i);
}
void CRTPReceive::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	string time="";
	time += to_string(t->tm_year + 1900) + "." + to_string(t->tm_mon + 1) + "." + to_string(t->tm_mday) + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(GetTickCount() % 1000) + "\n          ";
	
	fprintf(FileLogMixer, ("\n" +time + a + "\n//-------------------------------------------------------------------").c_str());
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
int CRTPReceive::decode_audio_frame(AVFrame *frame, int *data_present, int i)
{
	/** Packet used for temporary storage. */
	AVPacket input_packet;
	int error;
	init_packet(&input_packet);
	SHP_CAVPacket2 shpPacket;
#define async
#ifdef sync
	int szPack = vecSock[i]->receive_from(boost::asio::buffer(data, 8100), vecEndpoint[i]);
	if (szPack > 12)
	{
		rtp[i].reset(new CAVPacket2(12));
		shpPacket.reset(new CAVPacket2(szPack - 12));
		memcpy(rtp[i]->data, data, 12);
		memcpy(shpPacket->data, data + 12, szPack - 12);
	}
	else
	{
		shpPacket.reset(new CAVPacket2(0));
	}

#else //async
	if (vecData2[i].size > 12)
	{
		try
		{
			loggit("int CRTPReceive::decode_audio_frame bytes =" + to_string(vecData2[i].size));
			rtp[i].reset(new CAVPacket2(12));
			shpPacket.reset(new CAVPacket2(vecData2[i].size - 12));
			memcpy(shpPacket->data, vecData2[i].data + 12, vecData2[i].size - 12);//+12
			vecData2[i].size = 0;
			skipper[i] = false;
			time_start_receive[i] = std::chrono::high_resolution_clock::now();
		}
		catch (std::exception& e)
		{
			loggit("In decode_audio_frame: Exception:" + to_string(*e.what()));
			cerr << "Exception: " << e.what() << "\n";
			shpPacket.reset(new CAVPacket2(vecData[i].size-12));
			skipper[i] = true;
			time_start_receive[i] = std::chrono::high_resolution_clock::now();
		}
	}
	else
	{
		skipper[i] = true;
		loggit("no data from port " + to_string(vecEndpoint[i].port()) + " average =" + to_string(vecData[i].size - 12));
		shpPacket.reset(new CAVPacket2(vecData[i].size-12));
		time_start_receive[i] = std::chrono::high_resolution_clock::now();
	}
	
#endif
	if ((error = avcodec_decode_audio4(ext.iccx[i], frame, data_present, shpPacket.get())) < 0)
	{
		string s(get_error_text(error));
		loggit("Could not decode frame (error" + s);
		av_log(NULL, AV_LOG_ERROR, "Could not decode frame (error '%s')\n", get_error_text(error));
		av_free_packet(&input_packet);
		return error;
	}
	av_free_packet(&input_packet);
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CRTPReceive::encode_audio_frame(AVFrame *frame, int *data_present, int i)
{
	//loggit("int CRTPReceive::encode_audio_frame for i=" + to_string(i));
	AVPacket output_packet;
	int error;
	init_packet(&output_packet);

	if ((error = avcodec_encode_audio2(ext.out_iccx[i], &output_packet,
		frame, data_present)) < 0) 
	{
		string s(get_error_text(error));
		loggit("Could not encode frame (error " + s);
		av_log(NULL, AV_LOG_ERROR, "Could not encode frame (error '%s')\n", get_error_text(error));
		av_free_packet(&output_packet);
		return error;
	}
	auto timenow = std::chrono::high_resolution_clock::now();
	if (std::chrono::milliseconds(20) > (timenow - time_start_receive[i]))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(20) - (timenow - time_start_receive[i]));
	}//problem
	
	try
	{
		send.reset(new CAVPacket2(output_packet.size + 12));
		memcpy(send->data, rtp[i]->data, 12);
		memcpy(send->data + 12, output_packet.data, output_packet.size);
		loggit("sending " + to_string(send->size) + "bytes to ip " + vecEndpoint[i].address().to_string() + " and port=" + to_string(vecEndpoint[i].port()));
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - time_start_receive[i]);
		loggit("it took " + to_string(time_span.count()));
		vecSock[i]->send_to(boost::asio::buffer(send->data, send->size), vecEndpoint[i]);
		av_free_packet(&output_packet);
	}
	catch (std::exception& e)
	{

		loggit("encode_audio_frame: Exception:" + to_string(*e.what()) + " in thread i=" + to_string(i) + " (Inactive socket skiped port )" + to_string(vecEndpoint[i].port()));
		av_free_packet(&output_packet);
	}
	

	
	//loggit("int CRTPReceive::encode_audio_frame END");
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CRTPReceive::process_all()
{
	//loggit("int CRTPReceive::process_all() initing");
	int ret = 0;

	vector<int> data_present;

	for (int i = 0; i < tracks; ++i)
	{
		boost::shared_ptr<boost::thread> thread(new boost::thread(&StartReceive, this, i));
		receive_threads.push_back(thread);
		time_start_receive.push_back(std::chrono::high_resolution_clock::now());
		skipper.push_back(false);
		data_present.push_back(0);
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	loggit("int CRTPReceive::process_all() initing DONE");
	while ((process_all_running == true))
	{
		time_start_receive[0] = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < tracks; ++i)
		{
			AVFrame *frame = NULL;

			init_input_frame(&frame);
			/** Decode one frame worth of audio samples. */

			if ((ret = decode_audio_frame(frame, &data_present[i], i)))
			{
				loggit("if ((ret = decode_audio_frame(frame, ifcx[i], iccx[i], &data_present, &finished))) FAILED");
				//todo перезагрузка
			}

			//каждому клиенту в соответствующий буффер даем фрейм
			for (int j = 0; j < tracks; ++j) //вернуть
			{
				if (i == j) continue;

				if (j < i)
				{
					ret = av_buffersrc_write_frame(ext.afcx[j].src[i - 1], frame);
				}
				else
				{
					ret = av_buffersrc_write_frame(ext.afcx[j].src[i], frame);
				}
			}

			av_frame_free(&frame);
			//data_present_in_graph = data_present[i] | data_present_in_graph;
		}

		/* pull filtered audio from the filtergraph */
		for (int k = 0; k < tracks; ++k)/*****/ //вернуть
		{
			AVFrame *filt_frame = av_frame_alloc();

			ret = av_buffersink_get_frame(ext.sinkVec[k], filt_frame);

			if (ret < 0){ add_track(net_); }
			if (ret = encode_audio_frame(filt_frame, &data_present[k], k) < 0)
			{
				loggit("if (ret = encode_audio_frame(filt_frame, out_ifcx[k], out_iccx[k], &data_present)< 0) FAILED");
			}
			av_frame_unref(filt_frame);
			av_frame_free(&filt_frame);
		}/*****/
		time_start_receive[0] = std::chrono::high_resolution_clock::now();

	}
	process_all_finished = true;
	for (unsigned i = 0; i < vecSock.size(); ++i)
	{
		vecSock[i]->close();
		receive_threads[i]->~thread();
	}
	receive_threads.clear();
	loggit("Process all END");
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::add_track(NetworkData net)
{
	loggit("CRTPReceive::add_track " + to_string(net_.my_ports.size()));
	net_ = net;
	process_all_running = false;
	Initer->FreeSockFFmpeg();
	Initer.reset(new CMixInit(net));
	assert(process_all_finished = false);
	process_all_finished = false;
	process_all_running = true;
	ext = Initer->data;
	tracks= net_.input_SDPs.size();
	reinit_sockets(true);
	
	process_all();
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::reinit_sockets(bool mode)
{
	loggit("CRTPReceive::reinit_sockets");
	if (mode == true)
	{
		vecSock.clear();
		rtp.clear();
		vecEndpoint.clear();
		vecData.clear();
		vecData2.clear();
	}
	
	for (unsigned i = 0; i < net_.my_ports.size(); ++i)
	{
		SHP_Socket a;
		udp::endpoint sender_endpoint;
		a.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), net_.my_ports[i])));

		vecEndpoint.push_back(sender_endpoint);
		vecSock.push_back(a);
		SHP_CAVPacket2 c;
		rtp.push_back(c);

		Data dat1;
		Data dat2;
		dat2.size = 0;
		vecData.push_back(dat1);
		vecData2.push_back(dat2);
		vecData[i].size = 360;
		vecData2[i].size = 360;

	}
	loggit("CRTPReceive::reinit_socketsDONE");
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::receive(int i)
{
	if (process_all_finished == false)
	{
		try
		{
			int szPack = vecSock[i]->receive_from(boost::asio::buffer(vecData[i].data, 8000), vecEndpoint[i]);
			if (szPack > 12)
			{
				rtp[i].reset(new CAVPacket2(12));
				memcpy(rtp[i]->data, vecData[i].data, 12);
				memcpy(vecData2[i].data, vecData[i].data, szPack);//-12
				vecData2[i].size = szPack;//-12
				vecData[i].size = szPack;//-12

			}
			receive(i);
		}
		catch (std::exception& e)
		{
			//loggit("In receive: Exception:" + to_string(*e.what()) + "in thread i=" +to_string(i));
			//cerr << "Exception: " << e.what() << "\n";
			e;
			receive(i);
		}
	}
	else
	{
		loggit("receive i=" + to_string(i) + " stoped");
	}
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::destroy_all()
{
	loggit("CRTPReceive::destroy_all");
	process_all_running = false;
	Initer->FreeSockFFmpeg();
	int counter = 0;
	loggit("CRTPReceive::destroy_all DONE");
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