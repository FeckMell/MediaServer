#include "stdafx.h"
#include "Audio.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Audio::Audio(vector<SHP_CnfPoint> points_) : vecPoints(points_)
{
	filter.reset(new  Filter(vecPoints));
	CreateSilentFrame();
	for (int i = 0; i < (int)vecPoints.size(); ++i) vecPoints[i]->SetMaxTimesTook(vecPoints.size());
	Run();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::CreateSilentFrame()
{
	int mark;
	silentFrame = make_shared<CAVFrame>();
	SHP_CAVPacket shpPacket = make_shared<CAVPacket>(160);
	string str(160, 1);//string length 160, elements = ASCII(1)
	memcpy(shpPacket->Data(), str.c_str(), 160);
	avcodec_decode_audio4(vecPoints[0]->iccx, silentFrame->Get(), &mark, shpPacket->Get());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::Run()
{
	state = true;
	for (unsigned i = 0; i < vecPoints.size(); ++i)
	{
		vecPoints[i]->socket->s.async_receive_from(
			boost::asio::buffer(rawBuf.data, 1000), 
			vecPoints[i]->endPoint,
			boost::bind(&Audio::Receive, this, _1, _2, i)
			);
	}
	this_thread::sleep_for(chrono::milliseconds(500));
	eventThread.reset(new thread(&Audio::RunIO, this));
}
void Audio::RunIO()
{
	//vecPoints[0]->ioCnf.run();
	//while (state) vecPoints[0]->ioCnf.run_one();
	while (state) vecPoints[0]->ioCnf.run();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::Receive(boost::system::error_code ec_, size_t size_, int i_)
{
	if (/*(!ec_) &&*/ state)
	{
		if (size_ > 12)
		{
			SHP_CAVPacket shpPacket = std::make_shared<CAVPacket>(size_ - 12);
			memcpy(shpPacket->Data(), rawBuf.data + 12, size_ - 12);
			vecPoints[i_]->StoreFrame(Decode(shpPacket, i_));
			ProceedData(i_);
		}
		vecPoints[i_]->socket->s.async_receive_from(
			boost::asio::buffer(rawBuf.data, 1000),
			vecPoints[i_]->endPoint,
			boost::bind(&Audio::Receive, this, _1, _2, i_)
			);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_CAVFrame Audio::Decode(SHP_CAVPacket packet_, int i_)
{
	SHP_CAVFrame frame = make_shared<CAVFrame>();
	int mark;
	avcodec_decode_audio4(vecPoints[i_]->iccx, frame->Get(), &mark, packet_->Get());
	return frame;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::ProceedData(int i_)
{
	FillFilter(i_);
	EncodeAndSend(GetFrameFromFilter(i_), i_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::FillFilter(int i_)
{
	for (int j = 0; j < (int)vecPoints.size(); ++j)
	{
		if (i_ == j) continue;

		SHP_CAVFrame frame = vecPoints[j]->GetFrame();
		if (frame == nullptr) { frame = silentFrame; cout << "\ni=" << i_ << " sf"; }
		if (i_ < j){ av_buffersrc_write_frame(filter->data.afcx[i_][j - 1], frame->Get()); }
		else{ av_buffersrc_write_frame(filter->data.afcx[i_][j], frame->Get()); }
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_CAVFrame Audio::GetFrameFromFilter(int i_)
{
	SHP_CAVFrame frame = make_shared<CAVFrame>();
	av_buffersink_get_frame(filter->data.sinkVec[i_], frame->Get());
	return frame;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::EncodeAndSend(SHP_CAVFrame frame_, int i_)
{
	int mark;
	SHP_CAVPacket output_packet = make_shared<CAVPacket>();
	avcodec_encode_audio2(vecPoints[i_]->occx, output_packet->Get(), frame_->Get(), &mark);
	SHP_CAVPacket send = make_shared<CAVPacket>(output_packet->Size() + 12);
	memcpy(send->Data(), (uint8_t*)&vecPoints[i_]->rtp.Get(), 12);
	memcpy(send->Data() + 12, output_packet->Data(), output_packet->Size());
	try
	{
		vecPoints[i_]->socket->s.send_to(boost::asio::buffer(send->Data(), send->Size()), vecPoints[i_]->endPoint);
	}
	catch (std::exception& e)
	{
		cout << "\nAudio::EncodeAndSend Exeption:" << e.what();
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::MD(vector<SHP_CnfPoint> vecPoints_)
{
	vecPoints = vecPoints_;
	for (int i = 0; i < (int)vecPoints.size(); ++i) vecPoints[i]->SetMaxTimesTook(vecPoints.size());
	filter.reset(new Filter(vecPoints));
	Run();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::Stop()
{
	state = false;
	for (auto &e : vecPoints) e->socket->s.cancel();
	vecPoints[0]->ioCnf.reset();
	eventThread->join();
}
