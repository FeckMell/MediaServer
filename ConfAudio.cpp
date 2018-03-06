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
	create_silent_frame();
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
	while (process_all_finishing == false) callers_[0]->io_service_.run();
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
			memcpy(shpPacket->data(), callers_[i]->RawBuf.data + 12, szPack - 12);
			callers_[i]->RawBuf.data[0] = 0;
			callers_[i]->RawBuf.size = szPack - 12;
			shpPacket->make_size(szPack - 12);
			loggit("sz=" + to_string(szPack) + " reseive " + to_string(i) + " buf.size=" + to_string(callers_[i]->FrameBuf.size()));
			auto frame = decode(shpPacket, i);

			callers_[i]->FrameBuf.push(frame);

			proceed_data(i);


		}
		if (process_all_finishing == false)
		{
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
	// «апланируем получение на всех сокетах и выйдем
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







void ConfAudio::proceed_data(int i)
{
	fill_filter_for_i(i);
	SHP_CAVFrame frame = get_frame_from_filter(i);
	encode_and_send(frame, i);
}
//*/------------------------------------------------------------------------------------------
void ConfAudio::fill_filter_for_i(int i)
{
	for (int j = 0; j < (int)callers_.size(); ++j)
	{
		if (i == j) continue;

		SHP_CAVFrame frame = callers_[j]->FrameBuf.pop();
		//cout << "\ntest " << frame->empty() << " done";
		//cout << "\ntest2 " << SilentFrame->empty()<<" done"<<i;
		if (frame->empty())
		{ 
			//cout << "\nUSE SF "<<i;
			if (i < j){ av_buffersrc_write_frame(ext.afcx[i].src[j - 1], SilentFrame->get()); }
			else{ av_buffersrc_write_frame(ext.afcx[i].src[j], SilentFrame->get()); }
			//av_frame_unref(SilentFrame->get());
		}
		else{
			if (i < j){ av_buffersrc_write_frame(ext.afcx[i].src[j - 1], frame->get()); }
			else{ av_buffersrc_write_frame(ext.afcx[i].src[j], frame->get()); }
			//av_frame_unref(frame->get());
		}
		//cout << "\nfor " << i;
	}
	//cout << "\nfill end";
}
//*/------------------------------------------------------------------------------------------
SHP_CAVFrame ConfAudio::get_frame_from_filter(int i)
{
	SHP_CAVFrame frame = std::make_shared<CAVFrame>();
	av_buffersink_get_frame(ext.sinkVec[i], frame->get());
	return frame;
}
//*/------------------------------------------------------------------------------------------
void ConfAudio::encode_and_send(SHP_CAVFrame frame, int i)
{
	int mark;
	SHP_CAVPacket output_packet = std::make_shared<CAVPacket>();
	avcodec_encode_audio2(callers_[i]->out_iccx, output_packet->get(), frame->get(), &mark);
	SHP_CAVPacket send = std::make_shared<CAVPacket>(output_packet->size() + 12);
	callers_[i]->rtp.rtp_modify();
	memcpy(send->data(), (uint8_t*)&callers_[i]->rtp.header, 12);
	memcpy(send->data() + 12, output_packet->data(), output_packet->size());
	loggit("sending  " + to_string(send->size()) + "bytes to ip " + callers_[i]->Endpoint.address().to_string() + " and port=" + to_string(callers_[i]->Endpoint.port()));
	try
	{
		if (callers_[i]->Endpoint.address().to_string() == "0.0.0.0"){ loggit("inactive"); }
		else callers_[i]->Sock->send_to(boost::asio::buffer(send->data(), send->size()), callers_[i]->Endpoint);
	}
	catch (std::exception& e)
	{
		loggit("encode_audio_frame: Exception:" + to_string(*e.what()) + " (Inactive socket skiped port )" + to_string(callers_[i]->Endpoint.port()));
	}
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
SHP_CAVFrame ConfAudio::decode(SHP_CAVPacket packet, int i)
{
	SHP_CAVFrame frame = std::make_shared<CAVFrame>();
	int mark = 0;
	avcodec_decode_audio4(callers_[i]->iccx, frame->get(), &mark, packet->get());
	return frame;
}
//*/------------------------------------------------------------------------------------------
void ConfAudio::create_silent_frame()
{
	loggit("Creating silent frame");
	int mark;
	SilentFrame = std::make_shared<CAVFrame>();
	SHP_CAVPacket shpPacket = std::make_shared<CAVPacket>(160);
	string str = "";
	for (int k = 0; k < 20; ++k) str += "aaaaaaaa";
	memcpy(shpPacket->data(), str.c_str(), 160);
	avcodec_decode_audio4(callers_[0]->iccx, SilentFrame->get(), &mark, shpPacket->get());
	//for (int i = 0; i < (int)callers_.size() - 1; ++i)
	//{
	//	cout << i;
	//	av_buffersrc_write_frame(ext.afcx[0].src[i], SilentFrame->get());
	//}
	//loggit("1");
	//cout << "\n1";
	//SHP_CAVFrame frame;
	//frame = SilentFrame;
	loggit("Creating silent frame DONE");
}
//*/------------------------------------------------------------------------------------------




int ConfAudio::decode_audio_frame(SHP_CAVFrame frame, int *data_present, int i)
{
	SHP_CAVPacket shpPacket;
	loggit("Creating silent frame for ip=" + callers_[i]->Endpoint.address().to_string() + " and port=" + to_string(callers_[i]->Endpoint.port()));
	string str = "";
	for (int k = 0; k < callers_[i]->RawBuf.size / 8; ++k) str += "aaaaaaaa";
	shpPacket.reset(new CAVPacket(160));
	memcpy(shpPacket->data(), str.c_str(), 160);
	avcodec_decode_audio4(callers_[i]->iccx, frame->get(), data_present, shpPacket->get());
	return 0;
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
int ConfAudio::encode_audio_frame(SHP_CAVFrame frame, int *data_present, int i)
{
	int error;
	SHP_CAVPacket output_packet = std::make_shared<CAVPacket>();

	if ((error = avcodec_encode_audio2(callers_[i]->out_iccx, output_packet->get(), frame->get(), data_present)) < 0)
	{
		loggit("Could not encode frame (error " + string(get_error_text(error)));
		return error;
	}

	try
	{
		SHP_CAVPacket send = std::make_shared<CAVPacket>(output_packet->size() + 12);
		callers_[i]->rtp.rtp_modify();
		memcpy(send->data(), (uint8_t*)&callers_[i]->rtp.header, 12);
		memcpy(send->data() + 12, output_packet->data(), output_packet->size());
		loggit("sending " + to_string(send->size()) + "bytes to ip " + callers_[i]->Endpoint.address().to_string() + " and port=" + to_string(callers_[i]->Endpoint.port()));

		if (callers_[i]->Endpoint.address().to_string() == "0.0.0.0"){ loggit("inactive"); }
		else callers_[i]->Sock->send_to(boost::asio::buffer(send->data(), send->size()), callers_[i]->Endpoint);
	}
	catch (std::exception& e)
	{
		loggit("encode_audio_frame: Exception:" + to_string(*e.what()) + " (Inactive socket skiped port )" + to_string(callers_[i]->Endpoint.port()));
	}
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
	SHP_CAVFrame frame = std::make_shared<CAVFrame>();

	for (int k = 0; k < callers_[i]->RawBuf.size/8; ++k) str += "aaaaaaaa";
	shpPacket.reset(new CAVPacket(callers_[i]->RawBuf.size));
	memcpy(shpPacket->data(), str.c_str(), callers_[i]->RawBuf.size);//+12
	
	avcodec_decode_audio4(callers_[i]->iccx, frame->get(), &data_present, shpPacket->get());
	av_buffersrc_write_frame(ext.afcx[i].src[j], frame->get());

	//shpPacket->free();
	//frame->free();
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::get_last_buffer_frame(SHP_CAVFrame frame, int i)
{
	int ret;// , ret2;
	ret = av_buffersink_get_frame(ext.sinkVec[i], frame->get());
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::add_to_filter(int i, SHP_CAVFrame frame)
{
	for (int j = 0; j < (int)ext.afcx.size(); ++j)
	{
		if (i == j) continue;
		if (j < i){ av_buffersrc_write_frame(ext.afcx[j].src[i - 1], frame->get()); }
		else{ av_buffersrc_write_frame(ext.afcx[j].src[i], frame->get()); }
	}
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
int ConfAudio::process_all()
{
	loggit("process_all");
	receive();
	boost::shared_ptr<boost::thread> thread_io(new boost::thread(&ConfAudio::Run_io, this));
	//boost::shared_ptr<boost::thread> thread_pr(new boost::thread(&ConfAudio::new_process, this));

	//receive_threads.push_back(thread_pr);
	receive_threads.push_back(thread_io);
	loggit("process_all done");
	return 0;
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::new_process()
{
	loggit("PRE add to filter");
	//Mut_pr.lock();
	auto t1 = steady_clock::now();
	auto t2 = steady_clock::now();
	
	for (unsigned i = 0; i < callers_.size(); ++i)
	{
		SHP_CAVFrame frame = std::make_shared<CAVFrame>();
		SHP_CAVFrame filt_frame = std::make_shared<CAVFrame>();
		int data_present = 0;

		decode_audio_frame(frame, &data_present, i);
		add_to_filter(i, frame);

		av_frame_unref(filt_frame->get());
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
			SHP_CAVFrame frame = std::make_shared<CAVFrame>();
			SHP_CAVFrame filt_frame = std::make_shared<CAVFrame>();
			int data_present = 0;

			//decode_audio_frame(frame, &data_present, i);
			if (!callers_[i]->FrameBuf.empty())
				frame = callers_[i]->FrameBuf.pop();
			add_to_filter(i, frame);
			get_last_buffer_frame(filt_frame, i);
			encode_audio_frame(filt_frame, &data_present, i);

			av_frame_unref(filt_frame->get());
		}
	}
	loggit("new_process DONE");
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::clear_memmory()
{
	loggit("clearing");
	
	//receive_threads[0]->join();//new process
	for (auto &e : callers_)
	{
		e->Sock->cancel();
	}
	//receive_threads[1]->join();//run_io
	receive_threads[0]->join();//run_io
	callers_[0]->io_service_.reset();
	receive_threads.clear();
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
	clear_memmory();
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
