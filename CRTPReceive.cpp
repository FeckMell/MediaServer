#pragma once
#include "stdafx.h"
#include "Functions.h"
#include "CRTPReceive.h"
using namespace std;

void StartReceive(CRTPReceive* a, int i)
{
	//out << "\nTThread " << std::this_thread::get_id();
	a->receive(i);
}
void CRTPReceive::loggit(string a, int thread)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	string time = "";
	steady_clock::time_point t1 = steady_clock::now();
	time += to_string(t->tm_year + 1900) + "." + to_string(t->tm_mon + 1) + "." + to_string(t->tm_mday) + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	//fprintf(FileLogMixer, ("\n" + time + " thread=" + to_string(thread) + "       " + a).c_str());
	//fflush(FileLogMixer);
	CLogger.AddToLog(2, "\n" + time + " thread=" + to_string(thread) + "       " + a);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CRTPReceive::init_input_frame(AVFrame **frame)
{
	if (!(*frame = av_frame_alloc())) 
	{
		loggit("Could not allocate input frame", 9999);
		av_log(NULL, AV_LOG_ERROR, "Could not allocate input frame\n");
		return AVERROR(ENOMEM);
	}
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::init_packet(AVPacket *packet)
{
	av_init_packet(packet);
	/** Set the packet data and size so that it is recognized as being empty. */
	packet->data = NULL;
	packet->size = 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CRTPReceive::decode_audio_frame(AVFrame *frame, int *data_present, int i)
{
	int error;
	SHP_CAVPacket shpPacket;
	shpPacket->free();

	loggit("int CRTPReceive::decode_audio_frame bytes =" + to_string(vecData[i].size), i);
	try
	{
		shpPacket.reset(new CAVPacket(vecData[i].size - 12));
		memcpy(shpPacket->data, vecData[i].data + 12, vecData[i].size - 12);//+12
	}
	catch (std::exception& e)
	{
		loggit("decode_audio_frame: Exception:" + to_string(*e.what()) + " (Inactive socket skiped port )" + to_string(vecEndpoint[i].port()), i);
		exit(6);
	}
	
	if ((error = avcodec_decode_audio4(ext.iccx[i], frame, data_present, shpPacket.get())) < 0)
	{
		string s(get_error_text(error));
		loggit("Could not decode frame (error" + s, i);
		av_log(NULL, AV_LOG_ERROR, "Could not decode frame (error '%s')\n", get_error_text(error));
		return error;
	}
	mutex_.lock();
	av_frame_copy(vecFrame[i], frame);
	mutex_.unlock();
	shpPacket->free();
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CRTPReceive::encode_audio_frame(AVFrame *frame, int *data_present, int i)
{
	loggit("CRTPReceive::encode_audio_frame", i);
	AVPacket output_packet;
	SHP_CAVPacket send;
	send->free();
	int error;
	init_packet(&output_packet);
	loggit("CRTPReceive::encode_audio_frame initing done", i);
	if ((error = avcodec_encode_audio2(ext.out_iccx[i], &output_packet, frame, data_present)) < 0) 
	{
		string s(get_error_text(error));
		loggit("Could not encode frame (error " + s, i);
		av_log(NULL, AV_LOG_ERROR, "Could not encode frame (error '%s')\n", get_error_text(error));
		av_free_packet(&output_packet);
		return error;
	}

	try
	{
		send.reset(new CAVPacket(output_packet.size + 12));
		//memcpy(send->data, rtp[i]->data, 12);
		//rtp_modify(i);
		rtp2[i].rtp_modify();
		memcpy(send->data, (uint8_t*)&rtp2[i].header, 12);
		memcpy(send->data + 12, output_packet.data, output_packet.size);
		loggit("sending " + to_string(send->size) + "bytes to ip " + vecEndpoint[i].address().to_string() + " and port=" + to_string(vecEndpoint[i].port()), i);
		vecSock[i]->send_to(boost::asio::buffer(send->data, send->size), vecEndpoint[i]);

	}
	catch (std::exception& e)
	{

		loggit("encode_audio_frame: Exception:" + to_string(*e.what()) + " (Inactive socket skiped port )" + to_string(vecEndpoint[i].port()), i);
	}

	
	send->free();
	av_free_packet(&output_packet);
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CRTPReceive::process_all(NetworkData net)
{
	for (unsigned i = 0; i < vecSock.size(); ++i)
	{
		boost::shared_ptr<boost::thread> thread(new boost::thread(&StartReceive, this, i));
		receive_threads.push_back(thread);
	}
	loggit("Process all END", 9999);
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::receive(int i)
{
	loggit("void CRTPReceive::receive", i);
	while (true)
	{
		if (process_all_finishing == false)
		{
			try
			{
				int szPack = 0;
				szPack = vecSock[i]->receive_from(boost::asio::buffer(vecData[i].data, 8000), vecEndpoint[i]);
				if (szPack > 12)
				{
					loggit("received something>12", i);
					vecData[i].size = szPack;//-12
					szPack = 0;
					received = i;
					new_process(i);
					loggit("new_process DONE", i);
				}
				else
				{
					loggit("?????", i);
					exit(7);
				}


			}
			catch (std::exception& e)
			{
				loggit("In receive: Exception:" + to_string(*e.what()), i);
			}
		}
		else
		{
			loggit("receive stoped", i);
			return;
		}
	}
	
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::new_process(unsigned i)
{
	loggit("CRTPReceive::new_process START", i);
	AVFrame *frame = NULL;
	int ret, data_present = 0;
	init_input_frame(&frame);
	loggit("CRTPReceive::new_process initing DONE", i);
	decode_audio_frame(frame, &data_present, i);
	//каждому клиенту в соответствующий буффер даем фрейм
	for (unsigned j = 0; j < ext.afcx.size(); ++j)
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
	loggit("adding to buffer DONE", i);
	av_frame_free(&frame);
	AVFrame *filt_frame = av_frame_alloc();
	get_last_buffer_frame(filt_frame, i);
	
	if (ret = encode_audio_frame(filt_frame, &data_present, i) < 0)
	{
		loggit("if (ret = encode_audio_frame(filt_frame, &data_present[i], i) < 0) FAILED", i);
	}

	av_frame_unref(filt_frame);
	av_frame_free(&filt_frame);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
///////////////////////////////////////////
void CRTPReceive::add_missing_frame(int i, int j)
{
	int error;
	//SHP_CAVPacket shpPacket;
	//shpPacket->free();
	//AVFrame *frame = NULL;
	int data_present = 0;
	//init_input_frame(&frame);
	loggit("initing add_missing_frame DONE", i);
	/*shpPacket.reset(new CAVPacket(vecData[i].size - 12));
	for (int k = 0; k < vecData[i].size; ++k)
	{
		memcpy(shpPacket->data+k, "0", 1);//+12
	}
	loggit("reset packet", i);
	if ((error = avcodec_decode_audio4(ext.iccx[j], frame, &data_present, shpPacket.get())) < 0)//--//
	{
		loggit("shit",8888);
		//string s(get_error_text(error));
		//loggit("Could not decode frame (error" + s, i);
	}*/
	

	loggit("decode done, adding to buffer " + std::to_string(j), i);
	mutex_.lock();
	error = av_buffersrc_write_frame(ext.afcx[i].src[j], vecFrame[j]);//--//
	mutex_.unlock();
	loggit("adding to buffer DONE", i);
	//shpPacket->free();
	//av_frame_free(&frame);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
///////////////////////////
void CRTPReceive::get_last_buffer_frame(AVFrame* filt_frame, int i)
{
	loggit("CRTPReceive::get_last_buffer_frame", i);
	int ret, ret2;
	loggit("going to extract frame", i);
	ret = av_buffersink_get_frame(ext.sinkVec[i], filt_frame);
	loggit("frame extracted, checking fullness", i);
	if ((ret != AVERROR(EAGAIN)) && (ret != AVERROR_EOF))// если успешно изъ€ли берем следующий
	{
		loggit("extract of frame success", i);
		while (true)
		{
			loggit("while", i);
			AVFrame* frame_next = av_frame_alloc();
			ret2 = av_buffersink_get_frame(ext.sinkVec[i], frame_next);//--//
			loggit("ret2 = av_buffersink_get_frame", i);
			if (ret2 == AVERROR(EAGAIN) || ret2 == AVERROR_EOF)
			{
				loggit("it was last frame", i);
				av_frame_free(&frame_next);
				return;
			}
			else
			{
				loggit("there was extra frame", i);
				av_frame_unref(filt_frame);
				av_frame_copy(filt_frame, frame_next);
				av_frame_free(&frame_next);
			}
		}
	}
	else
	{
		loggit("extract new frame unsuccess", i);
		for (unsigned k = 0; k < ext.afcx[i].src.size(); ++k)
		{
			loggit("k=" + to_string(k), i);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			{
				for (unsigned j = 0; j < ext.afcx[i].src.size(); j++)
				{
					if (av_buffersrc_get_nb_failed_requests(ext.afcx[i].src[j]) > 0)//--//
					{
						loggit("adding frame for buffer j=" + to_string(j), i);
						add_missing_frame(i, j);
					}
				}
				loggit("extracting again", i);
				ret = av_buffersink_get_frame(ext.sinkVec[i], filt_frame);//--//
			}
			else
			{
				loggit("now we extracted frame", i);
				break;
			}
		}

	}

}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::clear_memmory()
{
	for (unsigned i = 0; i < vecData.size(); ++i)
	{
		//out << "\nreceive_threads[i]->~thread();";
		receive_threads[i]->~thread();
		vecSock[i]->close();
		vecData[i].free();
	}
	vecData.clear();
	receive_threads.clear();
	rtp2.clear();
	vecSock.clear();
	vecEndpoint.clear();
	net_.free();
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::add_track(NetworkData net)
{
	loggit("CRTPReceive::add_track " + to_string(net.my_ports.size()), 9999);
	process_all_finishing = true;
	for (unsigned i = 0; i < vecSock.size(); ++i)
	{
		receive_threads[i]->~thread();
		vecSock[i]->close();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	clear_memmory();
	Initer->FreeSockFFmpeg();
	net_ = net;
	Initer.reset(new CMixInit(net));

	ext = Initer->data;
	reinit_sockets(true);
	loggit("CRTPReceive::add_track END + process_all starter", 9999);
	process_all_finishing = false;
	process_all(net);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::reinit_sockets(bool mode)
{
	loggit("CRTPReceive::reinit_sockets", 9999);
	for (unsigned i = 0; i < net_.my_ports.size(); ++i)
	{
		SHP_Socket a;
		udp::endpoint sender_endpoint;
		a.reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), net_.my_ports[i])));

		vecEndpoint.push_back(sender_endpoint);
		vecSock.push_back(a);
		RTP_struct rtp_hdr;
		rtp_hdr.rtp_config();
		rtp2.push_back(rtp_hdr);

		Data dat1;
		vecData.push_back(dat1);

		AVFrame* frame;
		init_input_frame(&frame);
		vecFrame.push_back(frame);
	}
	loggit("CRTPReceive::reinit_socketsDONE", 9999);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::destroy_all()
{
	loggit("CRTPReceive::destroy_all", 9999);
	process_all_finishing = true;
	std::this_thread::sleep_for(std::chrono::milliseconds(800));
	clear_memmory();
	//out << "\nIniter->FreeSockFFmpeg();" << std::this_thread::get_id();
	Initer->FreeSockFFmpeg();
	loggit("CRTPReceive::destroy_all DONE", 9999);
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




int CRTPReceive::encode_audio_frame_file(AVFrame *frame, int *data_present, int i)
{
	loggit("int CRTPReceive::encode_audio_frame", i);
	AVPacket output_packet;
	int error;
	init_packet(&output_packet);

	if ((error = avcodec_encode_audio2(ext.out_iccx[i], &output_packet, frame, data_present)) < 0)
	{
		string s(get_error_text(error));
		loggit("Could not encode frame (error " + s, i);
		av_log(NULL, AV_LOG_ERROR, "Could not encode frame (error '%s')\n",
			get_error_text(error));
		av_free_packet(&output_packet);
		return error;
	}


	if (*data_present)
	{
		loggit("encode", i);
		if ((error = av_interleaved_write_frame(ext.out_ifcx[i], &output_packet)) < 0)
		{
			string s(get_error_text(error));
			loggit("Could not write frame (error " + s, i);
			av_log(NULL, AV_LOG_ERROR, "Could not write frame (error '%s')\n", get_error_text(error));
			av_free_packet(&output_packet);
			return error;
		}
		av_free_packet(&output_packet);
	}

	loggit("int CRTPReceive::encode_audio_frame END", i);
	return 0;
}