#pragma once
#include "stdafx.h"
#include "CRTPReceive.h"
#define _DEBUG_time
using namespace std;

void StartReceive(CRTPReceive* a, int i)
{
	a->receive(i);
}
/*void StartEndpoint(CRTPReceive* a, int i)
{
	a->set_endpoint(i);
}*/
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
		}
		catch (std::exception& e)
		{
			loggit("In decode_audio_frame: Exception:" + to_string(*e.what()));
			cerr << "Exception: " << e.what() << "\n";
			shpPacket.reset(new CAVPacket2(vecData[i].size-12));
			skipper[i] = true;
		}
	}
	else
	{
		skipper[i] = true;
		loggit("no data from port " + to_string(vecEndpoint[i].port()) + " average =" + to_string(vecData[i].size - 12));
		shpPacket.reset(new CAVPacket2(vecData[i].size-12));
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
	if (*finished && *data_present)
		*finished = 0;
#ifdef DEBUG_time
	time_start_receive[i] = std::chrono::high_resolution_clock::now();
#endif // DEBUG_time
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
	if (std::chrono::milliseconds(20) >(std::chrono::high_resolution_clock::now() - time_start_receive[0]))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(20) - (std::chrono::high_resolution_clock::now() - time_start_receive[0]));
	}
	if ((skipper[i]==false)) 
	{	
		try
		{
			send.reset(new CAVPacket2(output_packet.size + 12));
			memcpy(send->data, rtp[i]->data, 12);
			memcpy(send->data + 12, output_packet.data, output_packet.size);
			loggit("sending " + to_string(send->size) + "bytes to ip " + vecEndpoint[i].address().to_string() + " and port=" + to_string(vecEndpoint[i].port()));
			std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - time_start_receive[0]);
			loggit("it took " + to_string(time_span.count()));
			vecSock[i]->send_to(boost::asio::buffer(send->data, send->size), vecEndpoint[i]);
			av_free_packet(&output_packet);
		}
		catch (std::exception& e)
		{

			loggit("encode_audio_frame: Exception:" + to_string(*e.what()) + " in thread i=" + to_string(i) + " (Inactive socket skiped port )" + to_string(vecEndpoint[i].port()));
			av_free_packet(&output_packet);
		}
	}
	else
	{
		loggit("skipped ip " + vecEndpoint[i].address().to_string() + " and port=" + to_string(vecEndpoint[i].port()));
	}
	
	//loggit("int CRTPReceive::encode_audio_frame END");
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CRTPReceive::process_all()
{
	loggit("int CRTPReceive::process_all() initing");
	int ret = 0;
	//int data_present = 0;
	int finished = 0;
	int total_out_samples = 0;
	int nb_finished = 0;

	vector<int> data_present;
	vector<int> input_finished;
	vector<int> input_to_read;
	vector<int> total_samples;
	for (int i = 0; i < tracks; ++i)
	{
		input_finished.push_back(0);
		input_to_read.push_back(1);
		total_samples.push_back(0);
		data_present.push_back(0);

	}

	for (int i = 0; i < tracks; ++i)
	{
		boost::shared_ptr<boost::thread> thread(new boost::thread(&StartReceive, this, i));
		receive_threads.push_back(thread);
		time_start_receive.push_back(std::chrono::high_resolution_clock::now());
		skipper.push_back(false);
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	loggit("int CRTPReceive::process_all() initing DONE");
	while ((process_all_stopper == true))
	{
		//loggit(" in the start of while (nb_finished < tracks)");
		int data_present_in_graph = 0;
		time_start_receive[0] = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < tracks; ++i)
		{
			//loggit(" in the start of for (int i = 0; i < tracks; ++i) i=" + to_string(i));
			//if (input_finished[i] || input_to_read[i] == 0) continue; //не вернуть
			input_to_read[i] = 0;
			AVFrame *frame = NULL;

			if (init_input_frame(&frame) > 0) { std::cout << "\n 2.1"; goto end; }
			/** Decode one frame worth of audio samples. */

			if ((ret = decode_audio_frame(frame, &data_present[i], &finished, i)))
			{
				loggit("if ((ret = decode_audio_frame(frame, ifcx[i], iccx[i], &data_present, &finished))) FAILED");
				goto end;
			}

			/** If there is decoded data, convert and store it */
			/* push the audio data from decoded frame into the filtergraph */
			//каждому клиенту в соответствующий буффер даем фрейм

			for (int j = 0; j < tracks; ++j) //вернуть
			{
				if (i == j) continue;

				if (j < i)
				{
					//if (skipper[i] == true)
					//	ret = av_buffersrc_write_frame(ext.afcx[j].src[i - 1], NULL);
					//else
					ret = av_buffersrc_write_frame(ext.afcx[j].src[i - 1], frame);
				}
				else
				{
					//if (skipper[i] == true)
					//	ret = av_buffersrc_write_frame(ext.afcx[j].src[i], NULL);
					//else
					ret = av_buffersrc_write_frame(ext.afcx[j].src[i], frame);
				}
				/*if (ret < 0)
				{
				loggit("Error writing EOF null frame for input " + to_string(i));
				av_log(NULL, AV_LOG_ERROR, "Error writing EOF null frame for input %d\n", i);
				goto end;
				}*/
			}
			//}

			av_frame_free(&frame);
			data_present_in_graph = data_present[i] | data_present_in_graph;
		}
		if (data_present_in_graph)
		{
			/* pull filtered audio from the filtergraph */
			for (int k = 0; k < tracks; ++k)/*****/ //вернуть
			{
				//loggit("for (int k = 0; k < tracks; ++k), k= " + to_string(k));
				AVFrame *filt_frame = av_frame_alloc();
				//while (1)/**/
				//{

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
					//continue;
				}

				if (ret = encode_audio_frame(filt_frame, &data_present[k], k) < 0)
				{
					loggit("if (ret = encode_audio_frame(filt_frame, out_ifcx[k], out_iccx[k], &data_present)< 0) FAILED");
				}
				av_frame_unref(filt_frame);
				//	}/**/
				av_frame_free(&filt_frame);
			}/*****/
			time_start_receive[0] = std::chrono::high_resolution_clock::now();
		}
		else
		{
			if (std::chrono::milliseconds(20) >(std::chrono::high_resolution_clock::now() - time_start_receive[0]))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(20) - (std::chrono::high_resolution_clock::now() - time_start_receive[0]));
			}
			time_start_receive[0] = std::chrono::high_resolution_clock::now();
			loggit("No data in graph\n");
			av_log(NULL, AV_LOG_INFO, "No data in graph\n");
			for (int i = 0; i < tracks; i++)
			{
				input_to_read[i] = 1;
			}
		}
	}
	process_all_finished = true;
	cout << "Process all END";
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
	process_all_stopper = false;
	Init->freesock();
	Init.reset(new CMixInit(input_SDPs, IPs, my_ports, remote_ports));
	long int counter=0;
	while (process_all_finished == false)
	{
		++counter; 
		if (counter % 500 == 0) { cout << "wait1" << process_all_stopper; }
	}
	loggit("waited for " + to_string(counter) + "iter");
	counter = 0;
	process_all_finished = false;
	process_all_stopper = true;
	ext = Init->data;
	while (sockets_stoped != tracks) { cout <<"t="<< tracks << " s=" << sockets_stoped; }
	sockets_stoped = 0;
	tracks= input_SDPs.size();
	reinit_sockets(my_ports, true);
	
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
void CRTPReceive::reinit_sockets(vector<int> my_ports, bool mode)
{
	if (mode == true)
	{
		for (unsigned int i = 0; i < vecSock.size(); ++i)
		{
			vecSock[i]->close();
		}
		vecSock.clear();
		rtp.clear();
		vecEndpoint.clear();
		vecData.clear();
		vecData2.clear();
	}
	
	for (int i = 0; i < tracks; ++i)
	{
		SHP_Socket a;
		udp::endpoint sender_endpoint;
		a.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), my_ports[i])));
		//int size = a->receive_from(boost::asio::buffer(data, 8000), sender_endpoint);
		//average = size-12;
		
		//boost::thread thread(&StartEndpoint, this, i);
		//thread.detach();

		vecEndpoint.push_back(sender_endpoint);
		vecSock.push_back(a);
		SHP_CAVPacket2 c;
		rtp.push_back(c);
		//rtp[i].reset(new CAVPacket2(12));
		//memcpy(rtp[i]->data, data, 12);

		Data dat1;
		Data dat2;
		dat2.size = 0;
		vecData.push_back(dat1);
		vecData2.push_back(dat2);
		vecData[i].size = 360;
		vecData2[i].size = 360;

		//boost::shared_ptr<boost::thread> thread(new boost::thread(&StartReceive, this, i));
		//receive_threads.push_back(thread);
	}
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::receive(int i)
{
	if (process_all_stopper == true)
	{
		try
		{
			int szPack = vecSock[i]->receive_from(boost::asio::buffer(vecData[i].data, 8000), vecEndpoint[i]);
			if (szPack > 12)
			{
				rtp[i].reset(new CAVPacket2(12));
				//shpPacket.reset(new CAVPacket2(szPack - 12));
				memcpy(rtp[i]->data, vecData[i].data, 12);
				memcpy(vecData2[i].data, vecData[i].data, szPack);//-12
				vecData2[i].size = szPack;//-12
				vecData[i].size = szPack;//-12

			}
			receive(i);
		}
		catch (std::exception& e)
		{
			loggit("In receive: Exception:" + to_string(*e.what()) + "in thread i=" +to_string(i));
			cerr << "Exception: " << e.what() << "\n";
			receive(i);

		}
		
	}
	else
	{
		++sockets_stoped;
		loggit("receive i=" + to_string(i) + " stoped");
		cout << "\nreceive i=" << i << " stoped";
	}
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::destroy_all()
{
	process_all_stopper = false;
	Init->freesock();
	int counter = 0;
	while (process_all_finished == false){ ++counter; if (counter % 2000 == 0) { loggit("wait2"); cout << "wait2"; } }
	loggit("destroy all 1 waited for " + to_string(counter) + " iter");
	counter = 0;
	//process_all_finished = false;
	//process_all_stopper = true;
	while (sockets_stoped != tracks) { ++counter; if (counter % 2000 == 0) { loggit("wait2"); cout << "wait3"; } }
	loggit("destroy all 2 waited for " + to_string(counter) + " iter");
	sockets_stoped = 0;
	for (int i = 0; i < tracks; ++i)
	{
		vecSock[i]->close();
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

