#include "Audio.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Audio::Audio(vector<SHP_CnfPoint> points_) : vecPoints(points_)
{
	BOOST_LOG_SEV(lg, trace) << "Audio::Audio(...) call to filter.reset(new  Filter(vecPoints));";
	filter.reset(new  Filter(vecPoints));
	BOOST_LOG_SEV(lg, trace) << "Audio::Audio(...): filter.reset(new  Filter(vecPoints));->CreateSilentFrame();";
	CreateSilentFrame();
	BOOST_LOG_SEV(lg, trace) << "Audio::Audio(...): filter.reset(new  Filter(vecPoints));->for (int i = 0; i < (int)vecPoints.size(); ++i) vecPoints[i]->SetMaxTimesTook(vecPoints.size());";
	for (int i = 0; i < (int)vecPoints.size(); ++i) vecPoints[i]->SetMaxTimesTook(vecPoints.size());
	BOOST_LOG_SEV(lg, trace) << "Audio::Audio(...): Run();";
	Run();
	BOOST_LOG_SEV(lg, trace) << "Audio::Audio(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::CreateSilentFrame()
{
	BOOST_LOG_SEV(lg, trace) << "Audio::CreateSilentFrame()";
	int mark;
	silentFrame = make_shared<CAVFrame>();
	SHP_CAVPacket shpPacket = make_shared<CAVPacket>(160);
	string str(160, 1);//string length 160, elements = ASCII(1)
	memcpy(shpPacket->Data(), str.c_str(), 160);
	avcodec_decode_audio4(vecPoints[0]->iccx, silentFrame->Get(), &mark, shpPacket->Get());
	BOOST_LOG_SEV(lg, trace) << "Audio::CreateSilentFrame() DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::Run()
{
	BOOST_LOG_SEV(lg, trace) << "Audio::Run()";
	state = true;
	for (unsigned i = 0; i < vecPoints.size(); ++i)
	{
		BOOST_LOG_SEV(lg, debug) << "Audio::Run(): vecPoints[i]->socket->s.async_receive_from(...) for i=" << i;
		vecPoints[i]->socket->s.async_receive_from(
			boost::asio::buffer(rawBuf.data, 1000), 
			vecPoints[i]->endPoint,
			boost::bind(&Audio::Receive, this, _1, _2, i)
			);
	}
	this_thread::sleep_for(chrono::milliseconds(30));
	BOOST_LOG_SEV(lg, trace) << "Audio::Run(): eventThread.reset(new thread(&Audio::RunIO, this));";
	eventThread.reset(new thread(&Audio::RunIO, this));
	BOOST_LOG_SEV(lg, trace) << "Audio::Run() DONE";
}
void Audio::RunIO()
{
	BOOST_LOG_SEV(lg, debug) << "Audio::RunIO()";
	vecPoints[0]->ioCnf.reset();
	vecPoints[0]->ioCnf.run();
	//while (state) vecPoints[0]->ioCnf.run_one();
	//while (state) vecPoints[0]->ioCnf.run();//if commented saves from proc 25%
	BOOST_LOG_SEV(lg, debug) << "Audio::RunIO() DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::Receive(boost::system::error_code ec_, size_t size_, int i_)
{
	BOOST_LOG_SEV(lg, trace) << "Audio::Receive(...) for i=" << i_;
	if (/*(!ec_) &&*/ state)
	{
		if (size_ > 12)
		{
			SHP_CAVPacket shpPacket = std::make_shared<CAVPacket>(size_ - 12);
			BOOST_LOG_SEV(lg, debug) << "Audio::Receive(...): i=" << i_ << " packetsize=" << size_;
			memcpy(shpPacket->Data(), rawBuf.data + 12, size_ - 12);
			BOOST_LOG_SEV(lg, trace) << "Audio::Receive(...): memcpy(shpPacket->Data(), rawBuf.data + 12, size_ - 12); DONE";
			vecPoints[i_]->StoreFrame(Decode(shpPacket, i_));
			BOOST_LOG_SEV(lg, trace) << "Audio::Receive(...): vecPoints[i_]->StoreFrame(Decode(shpPacket, i_)); DONE";
			ProceedData(i_);
		}
		BOOST_LOG_SEV(lg, trace) << "Audio::Receive(...): vecPoints[i_]->socket->s.async_receive_from(...)";
		vecPoints[i_]->socket->s.async_receive_from(
			boost::asio::buffer(rawBuf.data, 1000),
			vecPoints[i_]->endPoint,
			boost::bind(&Audio::Receive, this, _1, _2, i_)
			);
	}
	BOOST_LOG_SEV(lg, trace) << "Audio::Receive(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_CAVFrame Audio::Decode(SHP_CAVPacket packet_, int i_)
{
	BOOST_LOG_SEV(lg, debug) << "Audio::Decode(...) for i=" << i_;
	SHP_CAVFrame frame = make_shared<CAVFrame>();
	int mark;
	avcodec_decode_audio4(vecPoints[i_]->iccx, frame->Get(), &mark, packet_->Get());
	BOOST_LOG_SEV(lg, trace) << "Audio::Decode(...) for i=" << i_ << " DONE";
	return frame;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::ProceedData(int i_)
{
	BOOST_LOG_SEV(lg, trace) << "Audio::ProceedData(...) for i=" << i_;
	FillFilter(i_);
	BOOST_LOG_SEV(lg, trace) << "Audio::ProceedData(...): FillFilter(i_);->EncodeAndSend(GetFrameFromFilter(i_), i_);";
	EncodeAndSend(GetFrameFromFilter(i_), i_);
	BOOST_LOG_SEV(lg, trace) << "Audio::ProceedData(...) DONE";;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::FillFilter(int i_)
{
	BOOST_LOG_SEV(lg, trace) << "Audio::FillFilter(...) for i=" << i_;
	for (int j = 0; j < (int)vecPoints.size(); ++j)
	{
		if (i_ == j) continue;

		SHP_CAVFrame frame = vecPoints[j]->GetFrame();
		if (frame == nullptr) 
		{ 
			frame = silentFrame;
			BOOST_LOG_SEV(lg, debug) << "Audio::FillFilter(...) sf for i=" << i_;
		}
		if (i_ < j){ av_buffersrc_write_frame(filter->data.afcx[i_][j - 1], frame->Get()); }
		else{ av_buffersrc_write_frame(filter->data.afcx[i_][j], frame->Get()); }
	}
	BOOST_LOG_SEV(lg, trace) << "Audio::FillFilter(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_CAVFrame Audio::GetFrameFromFilter(int i_)
{
	BOOST_LOG_SEV(lg, trace) << "Audio::GetFrameFromFilter(...) for i=" << i_;
	SHP_CAVFrame frame = make_shared<CAVFrame>();
	av_buffersink_get_frame(filter->data.sinkVec[i_], frame->Get());
	BOOST_LOG_SEV(lg, trace) << "Audio::GetFrameFromFilter(...) DONE";
	return frame;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::EncodeAndSend(SHP_CAVFrame frame_, int i_)
{
	BOOST_LOG_SEV(lg, trace) << "Audio::EncodeAndSend(...) for i=" << i_;
	int mark;
	SHP_CAVPacket output_packet = make_shared<CAVPacket>();
	avcodec_encode_audio2(vecPoints[i_]->occx, output_packet->Get(), frame_->Get(), &mark);
	SHP_CAVPacket send = make_shared<CAVPacket>(output_packet->Size() + 12);
	memcpy(send->Data(), (uint8_t*)&vecPoints[i_]->rtp.Get(), 12);
	memcpy(send->Data() + 12, output_packet->Data(), output_packet->Size());
	BOOST_LOG_SEV(lg, trace) << "Audio::EncodeAndSend(...): packetSize=" << send->Size();
	try
	{
		vecPoints[i_]->socket->s.send_to(boost::asio::buffer(send->Data(), send->Size()), vecPoints[i_]->endPoint);
	}
	catch (std::exception& e)
	{
		BOOST_LOG_SEV(lg, fatal) << "Audio::EncodeAndSend Exeption:" << e.what();
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::MD(vector<SHP_CnfPoint> vecPoints_)
{
	BOOST_LOG_SEV(lg, trace) << "Audio::MD(...)";
	vecPoints = vecPoints_;
	for (int i = 0; i < (int)vecPoints.size(); ++i) vecPoints[i]->SetMaxTimesTook(vecPoints.size());
	BOOST_LOG_SEV(lg, trace) << "Audio::MD(...): filter.reset(new Filter(vecPoints));";
	filter.reset(new Filter(vecPoints));
	BOOST_LOG_SEV(lg, trace) << "Audio::MD(...): Run();";
	Run();
	BOOST_LOG_SEV(lg, trace) << "Audio::MD(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::Stop()
{
	BOOST_LOG_SEV(lg, trace) << "Audio::Stop()";
	state = false;
	//vecPoints[0]->ioCnf.reset();
	for (auto &e : vecPoints) e->socket->s.cancel();
	//reset here original
	BOOST_LOG_SEV(lg, debug) << "Audio::Stop(): eventThread->join();";
	eventThread->join();
	BOOST_LOG_SEV(lg, debug) << "Audio::Stop() DONE";
}
