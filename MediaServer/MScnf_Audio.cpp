#include "stdafx.h"
#include "MScnf_Audio.h"
using namespace cnf;


Audio::Audio(vector<SHP_Point> points_) : vecPoints(points_)
{
	LOG::Log(LOG::info, "CNF", "MSCNF:Audio(..) starting for points.size=" + to_string(vecPoints.size()));
	filter.reset(new  Filter(vecPoints));
	CreateSilentFrame();
	for (int i = 0; i < (int)vecPoints.size(); ++i) vecPoints[i]->SetMaxTimesTook(vecPoints.size());
	Run();
}
Audio::~Audio()
{
	LOG::Log(LOG::info, "CNF", "MSCNF: ~Audio finished");
	state = false;
	for (auto& e : vecPoints) e->socket->s.cancel();
	eventThread->join();
	filter.reset();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::CreateSilentFrame()
{
	int mark;
	silentFrame = make_shared<FRAME>();
	SHP_PACKET shpPacket = make_shared<PACKET>(160);
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
		vecPoints[i]->socket->AsyncReceive(boost::bind(&Audio::Receive, this, _1, _2, i));
	}

	eventThread.reset(new thread(&Audio::RunIO, this));
}
void Audio::RunIO()
{
	LOG::Log(LOG::info, "CNF", "MSCNF: Audio io run");
	vecPoints[0]->socket->io->reset();
	vecPoints[0]->socket->io->run();
	LOG::Log(LOG::info, "CNF", "MSCNF: Audio io finished");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::Receive(boost::system::error_code ec_, size_t size_, int i_)
{
	if (state)
	{
		if (size_ > 12)
		{
			SHP_PACKET shpPacket = std::make_shared<PACKET>(size_ - 12);
			memcpy(shpPacket->Data(), vecPoints[i_]->socket->buffer + 12, size_ - 12);
			vecPoints[i_]->StoreFrame(Decode(shpPacket, i_));
			ProceedData(i_);
		}
		vecPoints[i_]->socket->AsyncReceive(boost::bind(&Audio::Receive, this, _1, _2, i_));
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_FRAME Audio::Decode(SHP_PACKET packet_, int i_)
{
	SHP_FRAME frame = make_shared<FRAME>();
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

		SHP_FRAME frame = vecPoints[j]->GetFrame();
		if (frame == nullptr) { frame = silentFrame; }
		if (i_ < j){ av_buffersrc_write_frame(filter->data.afcx[i_][j - 1], frame->Get()); }
		else{ av_buffersrc_write_frame(filter->data.afcx[i_][j], frame->Get()); }
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_FRAME Audio::GetFrameFromFilter(int i_)
{
	SHP_FRAME frame = make_shared<FRAME>();
	av_buffersink_get_frame(filter->data.sinkVec[i_], frame->Get());
	return frame;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::EncodeAndSend(SHP_FRAME frame_, int i_)
{
	int mark;
	SHP_PACKET output_packet = make_shared<PACKET>();
	avcodec_encode_audio2(vecPoints[i_]->occx, output_packet->Get(), frame_->Get(), &mark);
	SHP_PACKET send = make_shared<PACKET>(output_packet->Size() + 12);
	memcpy(send->Data(), vecPoints[i_]->socket->GetRTP(), 12);
	memcpy(send->Data() + 12, output_packet->Data(), output_packet->Size());
	
	try{ vecPoints[i_]->socket->SendTo(send->Data(), send->Size()); }
	catch (std::exception& e){ e; }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------