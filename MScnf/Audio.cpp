#include "stdafx.h"
#include "Audio.h"
using namespace cnf;


Audio::Audio(vector<SHP_CnfPoint> points_) : vecPoints(points_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::Audio(...) call to filter.reset(new  Filter(vecPoints));";
	filter.reset(new  Filter(vecPoints));
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::Audio(...): filter.reset(new  Filter(vecPoints));->CreateSilentFrame();";
	CreateSilentFrame();
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::Audio(...): filter.reset(new  Filter(vecPoints));->for (int i = 0; i < (int)vecPoints.size(); ++i) vecPoints[i]->SetMaxTimesTook(vecPoints.size());";
	for (int i = 0; i < (int)vecPoints.size(); ++i) vecPoints[i]->SetMaxTimesTook(vecPoints.size());
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::Audio(...): Run();";
	Run();
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::Audio(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::CreateSilentFrame()
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::CreateSilentFrame()";
	int mark;
	silentFrame = make_shared<FRAME>();
	SHP_PACKET shpPacket = make_shared<PACKET>(160);
	string str(160, 1);//string length 160, elements = ASCII(1)
	memcpy(shpPacket->Data(), str.c_str(), 160);
	avcodec_decode_audio4(vecPoints[0]->iccx, silentFrame->Get(), &mark, shpPacket->Get());
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::CreateSilentFrame() DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::Run()
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::Run()";
	state = true;
	for (unsigned i = 0; i < vecPoints.size(); ++i)
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), debug) << "Audio::Run(): vecPoints[i]->socket->s.async_receive_from(...) for i=" << i;
		vecPoints[i]->socket->s.async_receive_from(
			boost::asio::buffer(rawBuf.data, 1000), 
			vecPoints[i]->endPoint,
			boost::bind(&Audio::Receive, this, _1, _2, i)
			);
	}
	this_thread::sleep_for(chrono::milliseconds(30));
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::Run(): eventThread.reset(new thread(&Audio::RunIO, this));";
	eventThread.reset(new thread(&Audio::RunIO, this));
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::Run() DONE";
}
void Audio::RunIO()
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), debug) << "Audio::RunIO()";
	vecPoints[0]->ioCnf.reset();
	vecPoints[0]->ioCnf.run();
	//while (state) vecPoints[0]->ioCnf.run_one();
	//while (state) vecPoints[0]->ioCnf.run();//if commented saves from proc 25%
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), debug) << "Audio::RunIO() DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::Receive(boost::system::error_code ec_, size_t size_, int i_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::Receive(...) for i=" << i_;
	if (/*(!ec_) &&*/ state)
	{
		if (size_ > 12)
		{
			SHP_PACKET shpPacket = std::make_shared<PACKET>(size_ - 12);
			BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), debug) << "Audio::Receive(...): i=" << i_ << " packetsize=" << size_;
			memcpy(shpPacket->Data(), rawBuf.data + 12, size_ - 12);
			BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::Receive(...): memcpy(shpPacket->Data(), rawBuf.data + 12, size_ - 12); DONE";
			vecPoints[i_]->StoreFrame(Decode(shpPacket, i_));
			BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::Receive(...): vecPoints[i_]->StoreFrame(Decode(shpPacket, i_)); DONE";
			ProceedData(i_);
		}
		BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::Receive(...): vecPoints[i_]->socket->s.async_receive_from(...)";
		vecPoints[i_]->socket->s.async_receive_from(
			boost::asio::buffer(rawBuf.data, 1000),
			vecPoints[i_]->endPoint,
			boost::bind(&Audio::Receive, this, _1, _2, i_)
			);
	}
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::Receive(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_FRAME Audio::Decode(SHP_PACKET packet_, int i_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), debug) << "Audio::Decode(...) for i=" << i_;
	SHP_FRAME frame = make_shared<FRAME>();
	int mark;
	avcodec_decode_audio4(vecPoints[i_]->iccx, frame->Get(), &mark, packet_->Get());
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::Decode(...) for i=" << i_ << " DONE";
	return frame;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::ProceedData(int i_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::ProceedData(...) for i=" << i_;
	FillFilter(i_);
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::ProceedData(...): FillFilter(i_);->EncodeAndSend(GetFrameFromFilter(i_), i_);";
	EncodeAndSend(GetFrameFromFilter(i_), i_);
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::ProceedData(...) DONE";;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::FillFilter(int i_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::FillFilter(...) for i=" << i_;
	for (int j = 0; j < (int)vecPoints.size(); ++j)
	{
		if (i_ == j) continue;

		SHP_FRAME frame = vecPoints[j]->GetFrame();
		if (frame == nullptr) 
		{ 
			frame = silentFrame;
			BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), debug) << "Audio::FillFilter(...) sf for i=" << i_;
		}
		if (i_ < j){ av_buffersrc_write_frame(filter->data.afcx[i_][j - 1], frame->Get()); }
		else{ av_buffersrc_write_frame(filter->data.afcx[i_][j], frame->Get()); }
	}
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::FillFilter(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_FRAME Audio::GetFrameFromFilter(int i_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::GetFrameFromFilter(...) for i=" << i_;
	SHP_FRAME frame = make_shared<FRAME>();
	av_buffersink_get_frame(filter->data.sinkVec[i_], frame->Get());
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::GetFrameFromFilter(...) DONE";
	return frame;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::EncodeAndSend(SHP_FRAME frame_, int i_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::EncodeAndSend(...) for i=" << i_;
	int mark;
	SHP_PACKET output_packet = make_shared<PACKET>();
	avcodec_encode_audio2(vecPoints[i_]->occx, output_packet->Get(), frame_->Get(), &mark);
	SHP_PACKET send = make_shared<PACKET>(output_packet->Size() + 12);
	memcpy(send->Data(), (uint8_t*)&vecPoints[i_]->rtp.Get(), 12);
	memcpy(send->Data() + 12, output_packet->Data(), output_packet->Size());
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::EncodeAndSend(...): packetSize=" << send->Size();
	try
	{
		vecPoints[i_]->socket->s.send_to(boost::asio::buffer(send->Data(), send->Size()), vecPoints[i_]->endPoint);
	}
	catch (std::exception& e)
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), fatal) << "Audio::EncodeAndSend Exeption:" << e.what();
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::MD(vector<SHP_CnfPoint> vecPoints_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::MD(...)";
	vecPoints = vecPoints_;
	for (int i = 0; i < (int)vecPoints.size(); ++i) vecPoints[i]->SetMaxTimesTook(vecPoints.size());
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::MD(...): filter.reset(new Filter(vecPoints));";
	filter.reset(new Filter(vecPoints));
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::MD(...): Run();";
	Run();
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::MD(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::Stop()
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Audio::Stop()";
	state = false;
	//vecPoints[0]->ioCnf.reset();
	for (auto &e : vecPoints) e->socket->s.cancel();
	//reset here original
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), debug) << "Audio::Stop(): eventThread->join();";
	eventThread->join();
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), debug) << "Audio::Stop() DONE";
}
