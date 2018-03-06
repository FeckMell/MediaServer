#pragma once
#include "stdafx.h"
#include "Functions.h"
#include "ConfAudio.h"
#define DEBUG_
using namespace std;
extern string PathEXE;

//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
ConfAudio::ConfAudio(vector<SHP_CConfPoint> callers, int ID) :callers_(callers), ID_(ID)
{
	int data_present = 0;
	loggit("ConfAudio constuct");
	SilentFrame.reset(new CAVFrame());
	process_all_finishing = false;
	Initer.reset(new CFilterInit(callers, ID_));
	ext = Initer->data;
	for (auto& e : callers_)
		e->FrameBuf.setAmount(callers.size());
	//loggit("1");
	SHP_CAVPacket shpPacket;
	//loggit("2");
	std::string str = "";
	for (int j = 0; j < 20; ++j) str += "aaaaaaaa";
	shpPacket.reset(new CAVPacket(160));
	//loggit("3");
	memcpy(shpPacket->data, str.c_str(), 160);
	//loggit("4");
	avcodec_decode_audio4(callers_[0]->iccx, SilentFrame->get(), &data_present, shpPacket.get());
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
	loggit("io_run START");
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
			int data_present = 0;
			SHP_CAVFrame shpFrame = std::make_shared<CAVFrame>();
			SHP_CAVPacket shpPacket = std::make_shared<CAVPacket>(szPack - 12);
			memcpy(shpPacket->data, callers_[i]->RawBuf.data + 12, szPack - 12);
			callers_[i]->RawBuf.data[0] = 0;
			callers_[i]->RawBuf.size = szPack - 12;
			shpPacket->size = szPack - 12;
			loggit("sz=" + to_string(szPack) + " receive " + to_string(i) + " buf.size=" + to_string(callers_[i]->FrameBuf.size()));
			loggit("a");
			avcodec_decode_audio4(callers_[i]->iccx, shpFrame->get(), &data_present, shpPacket.get());
			loggit("b");
			callers_[i]->FrameBuf.push(shpFrame);
			loggit("c");
			new_process(i);
		}
		if (process_all_finishing == false)
		{
			callers_[i]->Sock->async_receive_from(boost::asio::buffer(callers_[i]->RawBuf.data, 8000), callers_[i]->Endpoint,
				boost::bind(&ConfAudio::receive_h, this, _1, _2, i));
		}
		else loggit("receive stoped 1 for i=" + to_string(i));
	}
	else loggit("receive stoped 2 for i=" + to_string(i));
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
		if (callers_[i]->Endpoint.address().to_string() == "0.0.0.0"){ loggit("inactive " + std::to_string(i)); }
		else callers_[i]->Sock->send_to(boost::asio::buffer(send->data, send->size), callers_[i]->Endpoint);
		loggit("sending end");
	}
	catch (std::exception& e)
	{
		loggit("encode_audio_frame: Exception:" + to_string(*e.what()) + " (Inactive socket skiped port )" + to_string(callers_[i]->Endpoint.port()));
	}
	send->free();
	av_free_packet(&output_packet);
	loggit("encode end");
	return 0;
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::get_last_buffer_frame(AVFrame* filt_frame, int i)
{
	int ret;
	ret = av_buffersink_get_frame(ext.sinkVec[i], filt_frame);
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::add_to_filter(int i, int j, AVFrame* frame)
{
	if (j > i){ av_buffersrc_write_frame(ext.afcx[i].src[j - 1], frame); }
		else{ av_buffersrc_write_frame(ext.afcx[i].src[j], frame); }
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
int ConfAudio::process_all()
{
	loggit("process_all");
	receive();
	boost::shared_ptr<boost::thread> thread_io(new boost::thread(&ConfAudio::Run_io, this));
	receive_threads.push_back(thread_io);	
	loggit("process_all done");
	return 0;
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::new_process(int i)
{
	SHP_CAVFrame filt_frame = std::make_shared<CAVFrame>();
	int data_present = 0;
	loggit("1");
	for (int j = 0; j < (int)callers_.size(); ++j)
	{
		if (i == j) continue;
		loggit("1.1");
		SHP_CAVFrame frame = callers_[i]->FrameBuf.pop(i);
		if (frame->get()->pkt_size != 160){ loggit("use sf"); frame = SilentFrame; }
		loggit("1.2");
		add_to_filter(i, j, frame->get());
		loggit("1.3");
	}
	loggit("2");
	get_last_buffer_frame(filt_frame->get(), i);
	loggit("3");
	encode_audio_frame(filt_frame->get(), &data_present, i);
	loggit("4");
	av_frame_unref(filt_frame->get());
}
//*/------------------------------------------------------------------------------------------
//*/------------------------------------------------------------------------------------------
void ConfAudio::clear_memmory()
{
	loggit("clearing");
	
	
	receive_threads[0]->join();//new process
	callers_[0]->io_service_.reset();
	for (auto &e : callers_) e->Sock->cancel();
	
	
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
	for (auto &e : callers_)
	{
		e->FrameBuf.free();
		e->FrameBuf.setAmount(callers_.size());
	}
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
	for (auto &e : callers_)
		e->FrameBuf.free();
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
