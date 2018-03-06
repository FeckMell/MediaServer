#pragma once
#include "stdafx.h"
#include "Functions.h"
#include "CRTPReceive.h"
using namespace std;

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
//	int error;
	SHP_CAVPacket shpPacket;
	shpPacket->free();

	loggit("decode_audio_frame bytes =" + to_string(vecData[i].size) + "from ip=" + to_string(vecEndpoint[i][0].port()), i);
	try
	{
		shpPacket.reset(new CAVPacket(vecData[i].size - 12));
		memcpy(shpPacket->data, vecData[i].data + 12, vecData[i].size - 12);//+12
	}
	catch (std::exception& e)
	{
		loggit("decode_audio_frame: Exception:" + to_string(*e.what()) + " (Inactive socket skiped port )" + to_string(vecEndpoint[i][0].port()), i);
		exit(6);
	}
	avcodec_decode_audio4(ext.iccx[i], frame, data_present, shpPacket.get());

	shpPacket->free();
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
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
		loggit("sending " + to_string(send->size) + "bytes to ip " + vecEndpoint[i][0].address().to_string() + " and port=" + to_string(vecEndpoint[i][0].port()), i);
		vecSock[i]->send_to(boost::asio::buffer(send->data, send->size), vecEndpoint[i][0]);
		if (vecEndpoint[i].size() > 2)
		{
			loggit("sending " + to_string(send->size) + "bytes to ip " + vecEndpoint[i][1].address().to_string() + " and port=" + to_string(vecEndpoint[i][1].port()), i);
			vecSock[i]->send_to(boost::asio::buffer(send->data, send->size), vecEndpoint[i][1]);
		}
	}
	catch (std::exception& e)
	{

		loggit("encode_audio_frame: Exception:" + to_string(*e.what()) + " (Inactive socket skiped port )" + to_string(vecEndpoint[i][0].port()), i);
	}

	
	send->free();
	av_free_packet(&output_packet);
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::add_missing_frame(int i, int j)
{	
	int data_present = 0;
	string str = "";
	AVFrame* frame;
	SHP_CAVPacket shpPacket;

	for (int k = 0; k < vecData[i].size-12; ++k)
		str += "a";
	shpPacket->free();
	shpPacket.reset(new CAVPacket(vecData[i].size - 12));
	memcpy(shpPacket->data, vecData[i].data, vecData[i].size-12);//+12
	frame = av_frame_alloc();

	avcodec_decode_audio4(ext.iccx[i], frame, &data_present, shpPacket.get());
	mutex_.lock();
	av_buffersrc_write_frame(ext.afcx[i].src[j], frame);
	mutex_.unlock();

	av_frame_free(&frame);
	shpPacket->free();
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::get_last_buffer_frame(AVFrame* filt_frame, int i)
{
	loggit("CRTPReceive::get_last_buffer_frame", i);
	int ret, ret2;
	//oggit("going to extract frame", i);
	ret = av_buffersink_get_frame(ext.sinkVec[i], filt_frame);
	//oggit("frame extracted, checking fullness", i);
	if ((ret != AVERROR(EAGAIN)) && (ret != AVERROR_EOF))// если успешно изъ€ли берем следующий
	{
		loggit("extract of frame success", i);
		while (true)
		{
			//oggit("while", i);
			AVFrame* frame_next = av_frame_alloc();
			ret2 = av_buffersink_get_frame(ext.sinkVec[i], frame_next);//--//
			//oggit("ret2 = av_buffersink_get_frame", i);
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
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int CRTPReceive::process_all(NetworkData net)
{
	for (unsigned i = 0; i < vecSock.size(); ++i)
	{
		boost::shared_ptr<boost::thread> thread(new boost::thread(&CRTPReceive::receive, this, i));
		receive_threads.push_back(thread);
	}
	loggit("Process all END", 9999);
	return 0;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::receive(int i)
{
	loggit("void CRTPReceive::receive for ip=" + net_.IPs[i] + " and rem_port=" + std::to_string(net_.remote_ports[i]) + " and my_port=" + std::to_string(net_.my_ports[i]), i);
	while (true)
	{
		if (process_all_finishing == false)
		{
			try
			{
				int szPack = 0;
				szPack = vecSock[i]->receive_from(boost::asio::buffer(vecData[i].data, 8000), vecEndpoint[i][0]);
				if (szPack > 12)
				{
					loggit("received something>12", i);
					vecData[i].size = szPack;//-12
					if (vecEndpoint[i].size() > 1)
					{
						loggit("sending " + to_string(vecData[i].size) + "bytes to ip " + vecEndpoint[i][1].address().to_string() + " and port=" + to_string(vecEndpoint[i][1].port()), i);
						vecSock[i]->send_to(boost::asio::buffer(vecData[i].data, vecData[i].size), vecEndpoint[i][1]);
					}
					new_process(i);
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

	AVFrame *frame = av_frame_alloc();
	AVFrame *filt_frame = av_frame_alloc();
	int data_present = 0;

	decode_audio_frame(frame, &data_present, i);
	add_to_filter(i,frame);
	av_frame_free(&frame);
	get_last_buffer_frame(filt_frame, i);
	encode_audio_frame(filt_frame, &data_present, i);

	av_frame_unref(filt_frame);
	av_frame_free(&filt_frame);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::clear_memmory()
{
	for (unsigned i = 0; i < vecData.size(); ++i)
	{
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
	/*for (unsigned i = 0; i < vecSock.size(); ++i)
	{
	receive_threads[i]->~thread();
	vecSock[i]->close();
	}*/
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

		vector<udp::endpoint> vendp;
		vendp.push_back(sender_endpoint);
		vecEndpoint.push_back(vendp);

		vecSock.push_back(a);
		RTP_struct rtp_hdr;
		rtp_hdr.rtp_config();
		rtp2.push_back(rtp_hdr);

		Data dat1;
		vecData.push_back(dat1);

		AVFrame* frame;
		init_input_frame(&frame);
	}
	for (int i = (int)RecordNet_.my_ports.size() - 1; i >= 0; --i)
		add_endpoint(i, true);
	loggit("CRTPReceive::reinit_socketsDONE", 9999);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::destroy_all()
{
	loggit("CRTPReceive::destroy_all", 9999);
	process_all_finishing = true;
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	clear_memmory();
	//out << "\nIniter->FreeSockFFmpeg();" << std::this_thread::get_id();
	Initer->FreeSockFFmpeg();
	Initer.reset();
	RecordNet_.free();
	loggit("CRTPReceive::destroy_all DONE", 9999);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::add_endpoint_info(string new_ip, int new_port, int my_port, bool active)
{
	if (active)
	{
		int num = -1;
		for (unsigned i = 0; i < net_.my_ports.size(); ++i)
			if (net_.my_ports[i] == my_port){ num = i; break; }
		if (num == -1){ return; }
		RecordNet_.IPs.push_back(new_ip);
		RecordNet_.my_ports.push_back(my_port);
		RecordNet_.remote_ports.push_back(new_port);
		add_endpoint(RecordNet_.my_ports.size(), true);
		return;
	}
	else
		add_endpoint(RecordNet_.my_ports.size(), false);
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void CRTPReceive::add_endpoint(int i, bool active)
{
	if (active)
	{
		int num = -1;
		for (unsigned j = 0; j < net_.my_ports.size(); ++j)
			if (net_.my_ports[j] == RecordNet_.my_ports[i]){ num = i; break; }

		if (num != -1)
		{
			udp::endpoint endpt = udp::endpoint(boost::asio::ip::address::from_string(RecordNet_.IPs[i]), RecordNet_.remote_ports[i]);
			vecEndpoint[num].push_back(endpt);
			return;
		}
		else
		{
			RecordNet_.IPs.erase(RecordNet_.IPs.begin() + i);
			RecordNet_.remote_ports.erase(RecordNet_.remote_ports.begin() + i);
			RecordNet_.my_ports.erase(RecordNet_.my_ports.begin() + i);
			return;
		}
	}
	else
	{
		RecordNet_.IPs.erase(RecordNet_.IPs.begin() + i);
		RecordNet_.remote_ports.erase(RecordNet_.remote_ports.begin() + i);
		RecordNet_.my_ports.erase(RecordNet_.my_ports.begin() + i);
		return;
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
