#pragma once
#include "stdafx.h"
#include "Functions.h"
#include "ConfAudio.h"
#define DEBUG0
using namespace std;
extern string PathEXE;

//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
ConfAudio::ConfAudio(vector<SHP_CConfPoint> callers, int ID) :callers_(callers), ID_(ID)
{
	loggit("ConfAudio constuct");
	process_all_finishing = false;
	Initer.reset(new CFilterInit(callers, ID_));
	ext = Initer->data;
	/*outfile0.open(PathEXE+"ConfSocket0.wav", std::ofstream::binary);
	outfile1.open(PathEXE + "ConfSocket1.wav", std::ofstream::binary);
	outfile2.open(PathEXE + "ConfSocket2.wav", std::ofstream::binary);
	outfile3.open(PathEXE + "ConfSocket3.wav", std::ofstream::binary);
	outfile00.open(PathEXE + "ConfDecode0.wav", std::ofstream::binary);
	outfile11.open(PathEXE + "ConfDecode1.wav", std::ofstream::binary);
	outfile22.open(PathEXE + "ConfDecode2.wav", std::ofstream::binary);
	outfile33.open(PathEXE + "ConfDecode3.wav", std::ofstream::binary);
	outfile000.open(PathEXE + "ConfSend0.wav", std::ofstream::binary);
	outfile111.open(PathEXE + "ConfSend1.wav", std::ofstream::binary);
	outfile222.open(PathEXE + "ConfSend2.wav", std::ofstream::binary);
	outfile333.open(PathEXE + "ConfSend3.wav", std::ofstream::binary);*/

	loggit("ConfAudio constuct END");
	process_all();
	
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::loggit(string a)
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
void ConfAudio::Run_io()
{
	while (process_all_finishing == false)
		callers_[0]->io_service_.run();
	loggit("io_run DONE");
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::receive_h(boost::system::error_code ec, size_t szPack, int i)
{
	if ((!ec) && (process_all_finishing == false))
	{
		if (szPack > 12)
		{
			SHP_CAVPacket shpPacket = std::make_shared<CAVPacket>(szPack - 12);
			memcpy(shpPacket->data, callers_[i]->RawBuf.data + 12, szPack - 12);
			callers_[i]->RawBuf.data[0] = 0;
			callers_[i]->RawBuf.size = szPack - 12;
			shpPacket->size = szPack - 12;
			loggit("sz=" + to_string(szPack) + " reseive " + to_string(i) + " buf.size=" + to_string(callers_[i]->FrameBuf.size()));
			callers_[i]->FrameBuf.push(shpPacket);
			
			/*char* test = new char[szPack - 12];
			memcpy(test, callers_[i]->RawBuf.data + 12, szPack - 12);
			if (i == 0) { outfile0.write((char*)test, shpPacket->size); }
			else if (i == 1) { outfile1.write((char*)test, szPack - 12); }
			else if (i == 2) { outfile2.write((char*)test, szPack - 12); }
			else if (i == 3) { outfile3.write((char*)test, szPack - 12); }
			delete[]test;*/
			//szPack = 0;
		}
		if (process_all_finishing == false)
		{
			//receive_i(i);
			callers_[i]->Sock->async_receive_from(boost::asio::buffer(callers_[i]->RawBuf.data, 8000), callers_[i]->Endpoint,
				boost::bind(&ConfAudio::receive_h, this, _1, _2, i));
		}
		else
			loggit("receive stoped 1 for i="  + to_string(i));
	}
	else
		loggit("receive stoped 2 for i=" + to_string(i));
}
//*/------------------------------------------------------------------------------------------
void ConfAudio::receive()
{
	loggit("Receive all started!");
	// Запланируем получение на всех сокетах и выйдем
	if (process_all_finishing == false)
	{
		for (unsigned i = 0; i < callers_.size(); ++i)
		{
			loggit("Starting receive for ip=" + callers_[i]->remote_ip_ + " and port=" + to_string(callers_[i]->remote_port_) + "(i=" + to_string(i) + ")");
			callers_[i]->Sock->async_receive_from(boost::asio::buffer(callers_[i]->RawBuf.data, 8000), callers_[i]->Endpoint,
				boost::bind(&ConfAudio::receive_h, this, _1, _2, i));
		}
	}
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
int ConfAudio::init_input_frame(AVFrame **frame)
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
void ConfAudio::init_packet(AVPacket *packet)
{
	av_init_packet(packet);
	packet->data = NULL;
	packet->size = 0;
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
int ConfAudio::decode_audio_frame(AVFrame *frame, int *data_present, int i)
{
	SHP_CAVPacket shpPacket;
	if (callers_[i]->FrameBuf.empty())
	{
		loggit("Creating silent frame for ip=" + callers_[i]->Endpoint.address().to_string() + " and port=" + to_string(callers_[i]->Endpoint.port()));
		string str = "";
		for (int k = 0; k < callers_[i]->RawBuf.size / 8; ++k) str += "aaaaaaaa";
		shpPacket.reset(new CAVPacket(160));
		memcpy(shpPacket->data, str.c_str(), 160);
		
	}
	else
	{
		shpPacket = callers_[i]->FrameBuf.pop();
		loggit("decode_audio_frame bytes =" + to_string(shpPacket->size) + "from ip=" + callers_[i]->Endpoint.address().to_string() + " and port=" + to_string(callers_[i]->Endpoint.port()));
		/*if (i == 0) { outfile00.write((char*)shpPacket->data, shpPacket->size);  }
		else if (i == 1) { outfile11.write((char*)shpPacket->data, shpPacket->size);  }
		else if (i == 2) { outfile22.write((char*)shpPacket->data, shpPacket->size);  }
		else if (i == 3) { outfile33.write((char*)shpPacket->data, shpPacket->size);  }*/
	}
	avcodec_decode_audio4(callers_[i]->iccx, frame, data_present, shpPacket.get());
	shpPacket->free();
	return 0;
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
int ConfAudio::encode_audio_frame(AVFrame *frame, int *data_present, int i)
{
	int error;
	SHP_CAVPacket send;
	AVPacket output_packet;
	init_packet(&output_packet);

	if ((error = avcodec_encode_audio2(callers_[i]->out_iccx, &output_packet, frame, data_present)) < 0)
	{
		loggit("Could not encode frame (error " + string(get_error_text(error)));
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

		/*if (i == 0) { outfile000.write((char*)output_packet.data, output_packet.size);  }
		else if (i == 1) { outfile111.write((char*)output_packet.data, output_packet.size);  }
		else if (i == 2) { outfile222.write((char*)output_packet.data, output_packet.size);  }
		else if (i == 3) { outfile333.write((char*)output_packet.data, output_packet.size);  }*/
		if (callers_[i]->Endpoint.address().to_string() == "0.0.0.0"){ loggit("inactive"); }
		else callers_[i]->Sock->send_to(boost::asio::buffer(send->data, send->size), callers_[i]->Endpoint);
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
void ConfAudio::add_missing_frame(int i, int j)
{	
	loggit("add missing frame");
	int data_present = 0;
	string str = "";
	SHP_CAVPacket shpPacket;
	AVFrame* frame;
	frame = av_frame_alloc(); 

	for (int k = 0; k < callers_[i]->RawBuf.size/8; ++k) str += "aaaaaaaa";
	shpPacket.reset(new CAVPacket(callers_[i]->RawBuf.size));
	memcpy(shpPacket->data, str.c_str(), callers_[i]->RawBuf.size);//+12
	
	avcodec_decode_audio4(callers_[i]->iccx, frame, &data_present, shpPacket.get());
	av_buffersrc_write_frame(ext.afcx[i].src[j], frame);

	av_frame_free(&frame);
	shpPacket->free();
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::get_last_buffer_frame(AVFrame* filt_frame, int i)
{
	int ret;// , ret2;
	ret = av_buffersink_get_frame(ext.sinkVec[i], filt_frame);
	/*if ((ret != AVERROR(EAGAIN)) && (ret != AVERROR_EOF))// если успешно изъяли берем следующий
	{
		while (true)
		{
			AVFrame* frame_next = av_frame_alloc();
			ret2 = av_buffersink_get_frame(ext.sinkVec[i], frame_next);//--//
			if (ret2 == AVERROR(EAGAIN) || ret2 == AVERROR_EOF)
			{
				av_frame_free(&frame_next);
				return;
			}
			else
			{
				av_frame_unref(filt_frame);
				av_frame_copy(filt_frame, frame_next);
				av_frame_free(&frame_next);
			}
		}
	}
	else
	{
		for (unsigned k = 0; k < ext.afcx[i].src.size(); ++k)
		{
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			{
				for (unsigned j = 0; j < ext.afcx[i].src.size(); j++)
				{
					if (av_buffersrc_get_nb_failed_requests(ext.afcx[i].src[j]) > 0)
					{
						add_missing_frame(i, j);
					}
				}
				ret = av_buffersink_get_frame(ext.sinkVec[i], filt_frame);
			}
			else{break;	}
		}
	}*/

}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::add_to_filter(int i, AVFrame* frame)
{
	for (int j = 0; j < (int)ext.afcx.size(); ++j)
	{
		if (i == j) continue;
		if (j < i){	av_buffersrc_write_frame(ext.afcx[j].src[i - 1], frame);}
		else{av_buffersrc_write_frame(ext.afcx[j].src[i], frame);}
	}
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
int ConfAudio::process_all()
{
	loggit("process_all");
	receive();
	boost::shared_ptr<boost::thread> thread_io(new boost::thread(&ConfAudio::Run_io, this));
	boost::shared_ptr<boost::thread> thread_pr(new boost::thread(&ConfAudio::new_process, this));

	receive_threads.push_back(thread_pr);
	receive_threads.push_back(thread_io);
	loggit("process_all done");
	return 0;
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::new_process()
{
	loggit("PRE add to filter");
	Mut_pr.lock();
	auto t1 = steady_clock::now();
	auto t2 = steady_clock::now();
	
	for (unsigned i = 0; i < callers_.size(); ++i)
	{
		AVFrame *frame = av_frame_alloc();
		AVFrame *filt_frame = av_frame_alloc();
		int data_present = 0;

		decode_audio_frame(frame, &data_present, i);
		add_to_filter(i, frame);

		av_frame_unref(filt_frame);
		av_frame_free(&filt_frame);
		//av_frame_free(&frame);
	}
	loggit("PRE add to filter DONE");
	while (process_all_finishing == false)
	{
		t2 = steady_clock::now();
		auto dur = duration_cast<milliseconds>(t2 - t1);
		if (20 - dur.count() > 0)
		{
			std::this_thread::sleep_for(milliseconds(20 - dur.count() - 1));
		}
		t1 = steady_clock::now();
		if (process_all_finishing == true) { loggit("new_process BREAK"); break; }
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
	Mut_pr.unlock();
	loggit("new_process DONE");
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::clear_memmory()
{
	loggit("clearing");
	
	receive_threads[0]->join();//new process
	for (auto &e : callers_)
	{
		e->Sock->cancel();
	}
	receive_threads[1]->join();//run_io
	//receive_threads[1]->~thread();//run_io
	callers_[0]->io_service_.reset();
	
	Mut_io.lock();
	Mut_io.unlock();
	Mut_pr.lock();
	Mut_pr.unlock();
	receive_threads.clear();
	//Initer.reset();
	loggit("clearing DONE");
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::add_track(vector<SHP_CConfPoint> callers)
{

	loggit("ConfAudio::add_track ");
	process_all_finishing = true;
	clear_memmory();
	Initer.reset(new CFilterInit(callers, ID_));
	ext = Initer->data;
	callers_ = callers;
	process_all_finishing = false;
	loggit("ConfAudio::add_track END");
	process_all();
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::destroy_all()
{
	loggit("ConfAudio::destroy_all");
	process_all_finishing = true;
	//std::this_thread::sleep_for(std::chrono::milliseconds(30));
	clear_memmory();
	//Initer.reset();
	loggit("ConfAudio::destroy_all DONE");
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
