#pragma once
#include "stdafx.h"
#include "Functions.h"
#include "CRTPReceive.h"
using namespace std;
#define DEBUG1
void CRTPReceive::Run_io()
{
	while (process_all_finishing == false)
		io_service_.run();
		//io_service_.run_one();
	loggit("io_run DONE", 9999);
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::receive_h(boost::system::error_code ec, size_t szPack, int i)
{
	if ((!ec) && (process_all_finishing == false))
	{
		string f = boost::to_string(vecData[i].data);
		int sz = f.size();
		loggit("szPack=" + std::to_string(szPack) + " sz=" + std::to_string(sz), 8888 + i);
		if (szPack > 12)
		{
			SHP_CAVPacket shpPacket = std::make_shared<CAVPacket>(szPack - 12);
			memcpy(shpPacket->data, vecData[i].data + 12, szPack - 12);
			vecData[i].data[0] = 0;
			vecBuf[i].push(shpPacket);
#ifdef DEBUG0
			char* test = new char[szPack - 12];
			memcpy(test, callers_[i]->RawBuf.data + 12, szPack - 12);
			if (i == 0) { outfile0.write((char*)shpPacket->data, shpPacket->size); }
			else if (i == 1) { outfile1.write((char*)shpPacket->data, szPack - 12); }
			else if (i == 2) { outfile2.write((char*)shpPacket->data, szPack - 12); }
			else if (i == 3) { outfile3.write((char*)shpPacket->data, szPack - 12); }
			delete[]test;
#endif
			
		}
		if (process_all_finishing == false)
		{
			receive_i(i);
		}
		else
			loggit("receive stoped 1", 9999);
	}
	else
		loggit("receive stoped 2", 9999);
}

void CRTPReceive::receive()
{
	loggit("Receive all started!", 9999);
	// Запланируем получение на всех сокетах и выйдем
	if (process_all_finishing == false)
	{
		for (unsigned i = 0; i < vecSock.size(); ++i)
		{
			receive_i(i);
		}
	}
}

void CRTPReceive::receive_i(unsigned i)
{
	vecSock[i]->async_receive_from(boost::asio::buffer(vecData[i].data, 8000), vecEndpoint[i],
		boost::bind(&CRTPReceive::receive_h, this, _1, _2, i));
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::loggit(string a, int thread)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	string time = "";
	steady_clock::time_point t1 = steady_clock::now();
	time += DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	CLogger.AddToLog(2, "\n" + time + " thread=" + to_string(thread) + "       " + a);
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
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
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::init_packet(AVPacket *packet)
{
	av_init_packet(packet);
	/** Set the packet data and size so that it is recognized as being empty. */
	packet->data = NULL;
	packet->size = 0;
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
int CRTPReceive::decode_audio_frame(AVFrame *frame, int *data_present, int i)
{
//	int error;
	if (vecBuf[i].empty())
	{
		loggit("Creating silent frame for ip=" + to_string(vecEndpoint[i].port()), i);
		string str = "";
		SHP_CAVPacket shpPacket;

		for (int k = 0; k < /*vecData[i].size-12*/5; ++k)
			str += "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
		shpPacket->free();
		shpPacket.reset(new CAVPacket(/*vecData[i].size-12*/160));
		memcpy(shpPacket->data, str.c_str(), /*vecData[i].size-12*/160);//+12
		avcodec_decode_audio4(ext.iccx[i], frame, data_present, shpPacket.get());
		shpPacket->free();
	}
	else
	{
		SHP_CAVPacket shpPacket = vecBuf[i].pop();
		loggit("decode_audio_frame bytes =" + to_string(shpPacket->size) + "from ip=" + to_string(vecEndpoint[i].port()), i);
		avcodec_decode_audio4(ext.iccx[i], frame, data_present, shpPacket.get());
		shpPacket->free();
	}
	return 0;
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
int CRTPReceive::encode_audio_frame(AVFrame *frame, int *data_present, int i)
{
	//oggit("CRTPReceive::encode_audio_frame", i);
	AVPacket output_packet;
	SHP_CAVPacket send;
	send->free();
	int error;
	init_packet(&output_packet);
	//oggit("CRTPReceive::encode_audio_frame initing done", i);
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
		rtp2[i].rtp_modify();
		memcpy(send->data, (uint8_t*)&rtp2[i].header, 12);
		memcpy(send->data + 12, output_packet.data, output_packet.size);
		loggit("sending " + to_string(send->size) + "bytes to ip " + vecEndpoint[i].address().to_string() + " and port=" + to_string(vecEndpoint[i].port()), i);
#ifdef DEBUG1
		if (i == 0) { outfile0.write((char*)output_packet.data, output_packet.size); loggit("data" + to_string(i)); }
		else if (i == 1) { outfile1.write((char*)output_packet.data, output_packet.size); loggit("data" + to_string(i)); }
		else if (i == 2) { outfile2.write((char*)output_packet.data, output_packet.size); loggit("data" + to_string(i)); }
		else if (i == 3) { outfile3.write((char*)output_packet.data, output_packet.size); loggit("data" + to_string(i)); }
#endif
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
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::add_missing_frame(int i, int j)
{	
	int data_present = 0;
	string str = "";
	AVFrame* frame;
	SHP_CAVPacket shpPacket;

	for (int k = 0; k < /*vecData[i].size-12*/160; ++k)
		str += "a";
	shpPacket->free();
	shpPacket.reset(new CAVPacket(/*vecData[i].size-12*/160));
	memcpy(shpPacket->data, str.c_str(), /*vecData[i].size-12*/160);//+12
	frame = av_frame_alloc();

	avcodec_decode_audio4(ext.iccx[i], frame, &data_present, shpPacket.get());
	mutex_.lock();
	av_buffersrc_write_frame(ext.afcx[i].src[j], frame);
	mutex_.unlock();

	av_frame_free(&frame);
	shpPacket->free();
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::get_last_buffer_frame(AVFrame* filt_frame, int i)
{
	//loggit("CRTPReceive::get_last_buffer_frame", i);
	int ret, ret2;
	//oggit("going to extract frame", i);
	ret = av_buffersink_get_frame(ext.sinkVec[i], filt_frame);
	//oggit("frame extracted, checking fullness", i);
	if ((ret != AVERROR(EAGAIN)) && (ret != AVERROR_EOF))// если успешно изъяли берем следующий
	{
		//loggit("extract of frame success", i);
		while (true)
		{
			//oggit("while", i);
			AVFrame* frame_next = av_frame_alloc();
			ret2 = av_buffersink_get_frame(ext.sinkVec[i], frame_next);//--//
			//oggit("ret2 = av_buffersink_get_frame", i);
			if (ret2 == AVERROR(EAGAIN) || ret2 == AVERROR_EOF)
			{
				//loggit("it was last frame", i);
				av_frame_free(&frame_next);
				return;
			}
			else
			{
				//loggit("there was extra frame", i);
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
			//oggit("k=" + to_string(k), i);
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
				//oggit("extracting again", i);
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
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::add_to_filter(int i, AVFrame* frame)
{
	for (int j = 0; j < (int)ext.afcx.size(); ++j)
	{
		if (i == j) continue;

		if (j < i)
		{
			mutex_.lock();
			av_buffersrc_write_frame(ext.afcx[j].src[i - 1], frame);
			mutex_.unlock();
		}
		else
		{
			mutex_.lock();
			av_buffersrc_write_frame(ext.afcx[j].src[i], frame);
			mutex_.unlock();
		}
	}
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
int CRTPReceive::process_all(NetworkData net)
{
	try
	{
		loggit("process_all", 0);
		receive();
		//boost::shared_ptr<boost::thread> thread_re(new boost::thread(&CRTPReceive::receive, this));
		boost::shared_ptr<boost::thread> thread_pr(new boost::thread(&CRTPReceive::new_process, this));
		boost::shared_ptr<boost::thread> thread_io(new boost::thread(&CRTPReceive::Run_io, this));

		//receive_threads.push_back(thread_re);
		receive_threads.push_back(thread_pr);
		receive_threads.push_back(thread_io);
		loggit("process_all done", 0);
	}
	catch (std::exception& e)
	{
		loggit("Exeption process_all: " + boost::to_string(e.what()), 0);
	}
	return 0;
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::new_process()
{
	loggit("new_process", 0);
	auto t1 = steady_clock::now();
	auto t2 = steady_clock::now();
	while (process_all_finishing == false)
	{
		t2 = steady_clock::now();
		auto dur = duration_cast<milliseconds>(t2 - t1);
		std::this_thread::sleep_for(milliseconds(20 - dur.count()));
		t1 = steady_clock::now();
		for (unsigned i = 0; i < vecSock.size(); ++i)
		{
			loggit("start " + std::to_string(dur.count()), i);
			AVFrame *frame = av_frame_alloc();
			AVFrame *filt_frame = av_frame_alloc();
			int data_present = 0;

			decode_audio_frame(frame, &data_present, i);
			add_to_filter(i, frame);
			av_frame_free(&frame);
			get_last_buffer_frame(filt_frame, i);
			encode_audio_frame(filt_frame, &data_present, i);

			av_frame_unref(filt_frame);
			av_frame_free(&filt_frame);
		}
		
	}
	loggit("new_process DONE", 9999);
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::clear_memmory()
{
	loggit("clearing1", 9999);
	for (unsigned i = 0; i < vecSock.size(); ++i)
	{
		vecSock[i]->cancel();
		vecSock[i]->close();
	}	
	for (unsigned i = 0; i < receive_threads.size(); ++i)
		receive_threads[i]->~thread();
	vecBuf.clear();
	vecSock.clear();
	receive_threads.clear();

	vecData.clear();
	rtp2.clear();
	vecEndpoint.clear();
	net_.free();
	loggit("clearing DONE", 9999);
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::add_track(NetworkData net)
{
	try
	{
		loggit("CRTPReceive::add_track " + to_string(net.my_ports.size()), 9999);
		process_all_finishing = true;
		std::this_thread::sleep_for(std::chrono::milliseconds(30));

		Initer->FreeSockFFmpeg();
		io_service_.reset();
		clear_memmory();
		net_ = net;
		
		reinit_sockets(true);
		Initer.reset(new CMixInit(net));
		ext = Initer->data;
		loggit("CRTPReceive::add_track END + process_all starter", 9999);
		process_all_finishing = false;
		process_all(net);
	}
	catch (boost::exception& e)
	{
		std::cerr << boost::diagnostic_information(e);
		loggit(boost::diagnostic_information(e), 9999);
		//system("pause");
	}
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::reinit_sockets(bool mode)
{
	loggit("CRTPReceive::reinit_sockets", 9999);
	int size = net_.my_ports.size();
	vecSock.resize(size);
	vecEndpoint.resize(size);
	rtp2.resize(size);
	vecData.resize(size);
	for (int i = 0; i < size; ++i)
	{
		vecSock[i].reset(new udp::socket(io_service_, udp::endpoint(udp::v4(), net_.my_ports[i])));
		rtp2[i].rtp_config();
		CThreadedCircular buf(10);
		vecBuf.push_back(buf);
	}
	loggit("CRTPReceive::reinit_socketsDONE", 9999);
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::destroy_all()
{
	loggit("CRTPReceive::destroy_all", 9999);
	process_all_finishing = true;
	std::this_thread::sleep_for(std::chrono::milliseconds(25));
	clear_memmory();
	Initer->FreeSockFFmpeg();
	Initer.reset();
	//RecordNet_.free();
	loggit("CRTPReceive::destroy_all DONE", 9999);
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
