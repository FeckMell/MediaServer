#pragma once
#include "stdafx.h"
#include "Functions.h"
#include "CRTPReceive.h"
#define DEBUG1
using namespace std;

//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
CRTPReceive::CRTPReceive(vector<SHP_CConfPoint> callers, int ID) :callers_(callers), ID_(ID)
{
	loggit("CRTPReceive constuct");
	av_log_set_level(0);
	av_register_all();
	avcodec_register_all();
	avfilter_register_all();
	avformat_network_init();
	/*vector<AVCodecContext*>iccx;
	vector<AVCodecContext*>out_iccx;
	for (auto&e : callers_)
	{
		iccx.push_back(e->iccx);
		out_iccx.push_back(e->out_iccx);
	}*/
	process_all_finishing = false;
	//Initer.reset(new CMixInit(iccx, out_iccx, ID_));
	Initer.reset(new CMixInit(callers_, ID_));
	ext = Initer->data;
	outfile0.open("ConfResult0.wav", std::ofstream::binary);
	outfile1.open("ConfResult1.wav", std::ofstream::binary);
	outfile2.open("ConfResult2.wav", std::ofstream::binary);
	outfile3.open("ConfResult3.wav", std::ofstream::binary);
	loggit("Shold create initer");

	
	process_all();
	loggit("CRTPReceive constuct END");
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::Run_io()
{
	while (process_all_finishing == false)
		callers_[0]->io_service_.run();
	loggit("io_run DONE");
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::receive_h(boost::system::error_code ec, size_t szPack, int i)
{
	if ((!ec) && (process_all_finishing == false))
	{
		if (szPack > 12)
		{
			SHP_CAVPacket shpPacket = std::make_shared<CAVPacket>(szPack - 12);
			memcpy(shpPacket->data, callers_[i]->RawBuf.data + 12, szPack - 12);
			//callers_[i]->RawBuf.data[0] = 0;
			callers_[i]->RawBuf.size = szPack - 12;
			callers_[i]->FrameBuf.push(shpPacket);
			loggit("sz="+to_string(szPack)+" reseive " + to_string(i));
#ifdef DEBUG0
			char* test = new char[szPack - 12];
			memcpy(test, callers_[i]->RawBuf.data + 12, szPack - 12);
			if (i == 0) { outfile0.write((char*)test, shpPacket->size);  }
			else if (i == 1) { outfile1.write((char*)test, szPack - 12); }
			else if (i == 2) { outfile2.write((char*)test, szPack - 12);  }
			else if (i == 3) { outfile3.write((char*)test, szPack - 12);  }
			delete[]test;
#endif
			szPack = 0;
		}
		if (process_all_finishing == false)
		{
			//receive_i(i);
			callers_[i]->Sock->async_receive_from(boost::asio::buffer(callers_[i]->RawBuf.data, 8000), callers_[i]->Endpoint,
				boost::bind(&CRTPReceive::receive_h, this, _1, _2, i));
		}
		else
			loggit("receive stoped 1");
	}
	else
		loggit("receive stoped 2");
}
//*/------------------------------------------------------------------------------------------
void CRTPReceive::receive()
{
	loggit("Receive all started!");
	// Запланируем получение на всех сокетах и выйдем
	if (process_all_finishing == false)
	{
		for (unsigned i = 0; i < callers_.size(); ++i)
		{
			//receive_i(i);
			callers_[i]->Sock->async_receive_from(boost::asio::buffer(callers_[i]->RawBuf.data, 8000), callers_[i]->Endpoint,
				boost::bind(&CRTPReceive::receive_h, this, _1, _2, i));
		}
	}
}
//*/------------------------------------------------------------------------------------------
void CRTPReceive::receive2(int i)
{
	while (process_all_finishing == false)
	{
		size_t szPack = callers_[i]->Sock->receive_from(boost::asio::buffer(callers_[i]->RawBuf.data, 8000), callers_[i]->Endpoint);
		loggit("szPack=" + to_string(szPack));
		if (szPack > 12)
		{
			SHP_CAVPacket shpPacket = std::make_shared<CAVPacket>(szPack - 12);
			memcpy(shpPacket->data, callers_[i]->RawBuf.data + 12, szPack - 12);
			callers_[i]->RawBuf.data[0] = 0;
			callers_[i]->RawBuf.size = szPack - 12;
			callers_[i]->FrameBuf.push(shpPacket);
		}
	}
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	steady_clock::time_point t1 = steady_clock::now();
	string result = DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	result += " ID=" + to_string(ID_) + " thread=" + boost::to_string(this_thread::get_id()) + "      ";
	CLogger.AddToLog(2, "\n" + result + a);
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
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
	if (callers_[i]->FrameBuf.empty())
	{
		loggit("Creating silent frame for ip=" + to_string(callers_[i]->Endpoint.port()));
		string str = "";
		SHP_CAVPacket shpPacket;

		for (int k = 0; k < /*vecData[i].size-12*/5; ++k)
			str += "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
		shpPacket->free();
		shpPacket.reset(new CAVPacket(/*vecData[i].size-12*/160));
		memcpy(shpPacket->data, str.c_str(), /*vecData[i].size-12*/160);//+12
		//avcodec_decode_audio4(callers_[i]->iccx, frame, data_present, shpPacket.get());
		avcodec_decode_audio4(ext.iccx[i], frame, data_present, shpPacket.get());

		shpPacket->free();
	}
	else
	{
		SHP_CAVPacket shpPacket = callers_[i]->FrameBuf.pop();
		loggit("decode_audio_frame bytes =" + to_string(shpPacket->size) + "from ip=" + to_string(callers_[i]->Endpoint.port()));
		//avcodec_decode_audio4(callers_[i]->iccx, frame, data_present, shpPacket.get());
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
	//if ((error = avcodec_encode_audio2(callers_[i]->out_iccx, &output_packet, frame, data_present)) < 0)
	if ((error = avcodec_encode_audio2(ext.out_iccx[i], &output_packet, frame, data_present)) < 0)
	{
		string s(get_error_text(error));
		loggit("Could not encode frame (error " + s);
		av_log(NULL, AV_LOG_ERROR, "Could not encode frame (error '%s')\n", get_error_text(error));
		av_free_packet(&output_packet);
		return error;
	}

	try
	{
		send.reset(new CAVPacket(output_packet.size + 12));
		callers_[i]->rtp.rtp_modify();
		memcpy(send->data, (uint8_t*)&callers_[i]->rtp.header, 12);
		memcpy(send->data + 12, output_packet.data, output_packet.size);
		loggit("sending " + to_string(send->size) + "bytes to ip " + callers_[i]->Endpoint.address().to_string() + " and port=" + to_string(callers_[i]->Endpoint.port()));
#ifdef DEBUG1
		if (i == 0) { outfile0.write((char*)output_packet.data, output_packet.size); loggit("data" + to_string(i)); }
		else if (i == 1) { outfile1.write((char*)output_packet.data, output_packet.size); loggit("data" + to_string(i)); }
		else if (i == 2) { outfile2.write((char*)output_packet.data, output_packet.size); loggit("data" + to_string(i)); }
		else if (i == 3) { outfile3.write((char*)output_packet.data, output_packet.size); loggit("data" + to_string(i)); }
#endif
		callers_[i]->Sock->send_to(boost::asio::buffer(send->data, send->size), callers_[i]->Endpoint);
	}
	catch (std::exception& e)
	{

		loggit("encode_audio_frame: Exception:" + to_string(*e.what()) + " (Inactive socket skiped port )" + to_string(callers_[i]->Endpoint.port()));
	}

	
	send->free();
	av_free_packet(&output_packet);
	return 0;
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::add_missing_frame(int i, int j)
{	
	loggit("add missing frame");
	int data_present = 0;
	string str = "";
	AVFrame* frame;
	SHP_CAVPacket shpPacket;

	for (int k = 0; k < callers_[i]->RawBuf.size/8; ++k)
		str += "aaaaaaaa";
	shpPacket->free();
	shpPacket.reset(new CAVPacket(callers_[i]->RawBuf.size));
	memcpy(shpPacket->data, str.c_str(), callers_[i]->RawBuf.size);//+12
	frame = av_frame_alloc();

	//avcodec_decode_audio4(callers_[i]->iccx, frame, &data_present, shpPacket.get());
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
	loggit("CRTPReceive::get_last_buffer_frame");
	int ret, ret2;
	//oggit("going to extract frame");
	ret = av_buffersink_get_frame(ext.sinkVec[i], filt_frame);
	//oggit("frame extracted, checking fullness");
	if ((ret != AVERROR(EAGAIN)) && (ret != AVERROR_EOF))// если успешно изъяли берем следующий
	{
		//loggit("extract of frame success", i);
		while (true)
		{
			//oggit("while");
			AVFrame* frame_next = av_frame_alloc();
			ret2 = av_buffersink_get_frame(ext.sinkVec[i], frame_next);//--//
			//oggit("ret2 = av_buffersink_get_frame");
			if (ret2 == AVERROR(EAGAIN) || ret2 == AVERROR_EOF)
			{
				//loggit("it was last frame");
				av_frame_free(&frame_next);
				return;
			}
			else
			{
				//loggit("there was extra frame");
				av_frame_unref(filt_frame);
				av_frame_copy(filt_frame, frame_next);
				av_frame_free(&frame_next);
			}
		}
	}
	else
	{
		loggit("extract new frame unsuccess");
		for (unsigned k = 0; k < ext.afcx[i].src.size(); ++k)
		{
			//oggit("k=" + to_string(k), i);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			{
				for (unsigned j = 0; j < ext.afcx[i].src.size(); j++)
				{
					if (av_buffersrc_get_nb_failed_requests(ext.afcx[i].src[j]) > 0)//--//
					{
						loggit("adding frame for buffer j=" + to_string(j));
						add_missing_frame(i, j);
					}
				}
				//oggit("extracting again", i);
				ret = av_buffersink_get_frame(ext.sinkVec[i], filt_frame);//--//
			}
			else
			{
				loggit("now we extracted frame");
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
int CRTPReceive::process_all()
{
	try
	{
		loggit("process_all");
		
		/*for (unsigned i = 0; i < callers_.size(); ++i)
		{
			boost::shared_ptr<boost::thread> thread_re(new boost::thread(&CRTPReceive::receive2, this,i));
			receive_threads.push_back(thread_re);
		}*/
		receive();//release
		boost::shared_ptr<boost::thread> thread_io(new boost::thread(&CRTPReceive::Run_io, this));
		boost::shared_ptr<boost::thread> thread_pr(new boost::thread(&CRTPReceive::new_process, this));
		
		receive_threads.push_back(thread_pr);
		receive_threads.push_back(thread_io);
		loggit("process_all done");
	}
	catch (std::exception& e)
	{
		loggit("Exeption process_all: " + boost::to_string(e.what()));
	}
	return 0;
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::new_process()
{
	loggit("new_process");
	//std::this_thread::sleep_for(milliseconds(40));
	auto t1 = steady_clock::now();
	auto t2 = steady_clock::now();
	while (process_all_finishing == false)
	{
		t2 = steady_clock::now();
		auto dur = duration_cast<milliseconds>(t2 - t1);
		//if (20 - dur.count() > 0)
		//{
		//	std::this_thread::sleep_for(milliseconds(20 - dur.count()));
		//}
		std::this_thread::sleep_for(milliseconds(20));
		t1 = steady_clock::now();
		if (process_all_finishing == true) { loggit("BREAK"); break; }
		loggit("Speed=" + to_string(dur.count()));
		for (unsigned i = 0; i < callers_.size(); ++i)
		{
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
	loggit("new_process DONE");
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::clear_memmory()
{
	loggit("clearing");
	callers_[0]->io_service_.reset();
	for (unsigned i = 0; i < receive_threads.size(); ++i)
		receive_threads[i]->~thread();
	receive_threads.clear();
	Initer->FreeSockFFmpeg();
	loggit("clearing DONE");
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::add_track(vector<SHP_CConfPoint> callers)
{
	try
	{
		loggit("CRTPReceive::add_track ");
		process_all_finishing = true;
		std::this_thread::sleep_for(std::chrono::milliseconds(22));
		callers_ = callers;
		clear_memmory();
		/*vector<AVCodecContext*>iccx;
		vector<AVCodecContext*>out_iccx;
		for (auto&e : callers_)
		{
			iccx.push_back(e->iccx);
			out_iccx.push_back(e->out_iccx);
		}*/
		//Initer.reset(new CMixInit(iccx, out_iccx, ID_));
		Initer.reset(new CMixInit(callers, ID_));
		ext = Initer->data;
		loggit("CRTPReceive::add_track END + process_all starter");
		process_all_finishing = false;
		process_all();
	}
	catch (boost::exception& e)
	{
		std::cerr << boost::diagnostic_information(e);
		loggit(boost::diagnostic_information(e));
	}
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::Freeze()
{
	loggit("CRTPReceive::Freeze ");
	process_all_finishing = true;
	std::this_thread::sleep_for(std::chrono::milliseconds(22));
	clear_memmory();
	loggit("CRTPReceive::Freeze DONE ");
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void CRTPReceive::destroy_all()
{
	loggit("CRTPReceive::destroy_all 1");
	cout << callers_[0]->CallID_;
	process_all_finishing = true;
	std::this_thread::sleep_for(std::chrono::milliseconds(22));
	clear_memmory();
	Initer.reset();
	loggit("CRTPReceive::destroy_all DONE");
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
