#include "stdafx.h"
#include "MScnf_Audio.h"
using namespace cnf;


Audio::Audio(vector<SHP_Caller> points_) : vecCallers(points_)
{
	//LOG::Log(LOG::info, "CNF", "MSCNF:Audio(..) starting for points.size=" + to_string(vecCallers.size()));
	filter.reset(new  Filter(vecCallers));
	CreateSilentFrame();
	for (int i = 0; i < (int)vecCallers.size(); ++i) vecCallers[i]->SetJitterSize(vecCallers.size());
	Run();
}
Audio::~Audio()
{
	//LOG::Log(LOG::info, "CNF", "MSCNF: ~Audio finished");
	state = false;
	for (auto& e : vecCallers) e->basePoint->socket->s.cancel();
	eventThread->join();
	filter.reset();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::CreateSilentFrame()
{
	int mark;
	silentFrame = make_shared<FRAME>();
	SHP_PACKET shpPacket = make_shared<PACKET>(80);
	string str(80, 1);//string length 80 (ptime=10), elements = ASCII(1)
	memcpy(shpPacket->Data(), str.c_str(), 80);
	avcodec_decode_audio4(vecCallers[0]->iccx, silentFrame->Get(), &mark, shpPacket->Get());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::Run()
{
	state = true;
	for (unsigned i = 0; i < vecCallers.size(); ++i)
	{
		vecCallers[i]->basePoint->socket->AsyncReceive(boost::bind(&Audio::Receive, this, _1, _2, i));
	}

	eventThread.reset(new thread(&Audio::RunIO, this));
}
void Audio::RunIO()
{
	//LOG::Log(LOG::info, "CNF", "MSCNF: Audio io run");
	vecCallers[0]->basePoint->socket->io->reset();
	vecCallers[0]->basePoint->socket->io->run();
	//LOG::Log(LOG::info, "CNF", "MSCNF: Audio io finished");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::Receive(boost::system::error_code ec_, size_t size_, int i_)
{
	if (state)
	{
		if (size_ > 12)
		{
			vecCallers[i_]->ptime = (size_ - 12) / 80;
			SHP_PACKET shpPacket = make_shared<PACKET>(vecCallers[i_]->basePoint->socket->buffer + 12, size_ - 12);
			//SHP_PACKET shpPacket = FFF::CreatePacket(vecCallers[i_]->socket->buffer + 12, size_ - 12);
			ProceedData(shpPacket, i_);
		}
		vecCallers[i_]->basePoint->socket->AsyncReceive(boost::bind(&Audio::Receive, this, _1, _2, i_));
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::ProceedData(SHP_PACKET packet_, int i_)
{
	PrepareData(packet_, i_);

	for (int k = 0; k < vecCallers[i_]->ptime; ++k) FillFilter(i_);

	EncodeAndSend(i_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::PrepareData(SHP_PACKET packet_, int i_)
{
	for (int j = 0; j < vecCallers[i_]->ptime; ++j)
	{
		//SHP_PACKET temp_packet = FFF::CreatePacket(packet_->Data() + j * 80, 80);
		SHP_PACKET temp_packet = make_shared<PACKET>(packet_->Data() + j * 80, 80);
		SHP_FRAME temp_frame = Decode(temp_packet, i_);
		SaveFrameToJitters(temp_frame, i_);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_FRAME Audio::Decode(SHP_PACKET packet_, int i_)
{
	SHP_FRAME frame = make_shared<FRAME>();
	int mark;
	avcodec_decode_audio4(vecCallers[i_]->iccx, frame->Get(), &mark, packet_->Get());
	return frame;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::SaveFrameToJitters(SHP_FRAME frame_, int from_which_)
{
	for (int i = 0; i < (int)vecCallers.size(); ++i)
	{
		if (i == from_which_) continue;

		vecCallers[i]->StoreFrame(frame_, from_which_);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::FillFilter(int i_)
{
	for (int j = 0; j < (int)vecCallers.size(); ++j)// from all other buffers
	{
		if (i_ == j) continue;

		SHP_FRAME frame = vecCallers[i_]->GetFrame(j);

		if (frame == nullptr) { frame = silentFrame; }
		if (i_ < j){ av_buffersrc_write_frame(filter->data.afcx[i_][j - 1], frame->Get()); }
		else{ av_buffersrc_write_frame(filter->data.afcx[i_][j], frame->Get()); }
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Audio::EncodeAndSend(int i_)
{
	SHP_PACKET result = make_shared<PACKET>();
	for (int j = 0; j < vecCallers[i_]->ptime; ++j)
	{
		SHP_PACKET temp_packet = make_shared<PACKET>();
		SHP_FRAME temp_frame = GetFrameFromFilter(i_);
		int mark;
		avcodec_encode_audio2(vecCallers[i_]->occx, temp_packet->Get(), temp_frame->Get(), &mark);
		result = make_shared<PACKET>(result.get(), temp_packet.get());
	}
	SHP_PACKET rtp_header = make_shared<PACKET>(vecCallers[i_]->basePoint->socket->GetRTP(vecCallers[i_]->ptime), 12);
	rtp_header = make_shared<PACKET>(rtp_header.get(), result.get());

	try{ vecCallers[i_]->basePoint->socket->SendTo(rtp_header->Data(), rtp_header->Size()); }
	catch (std::exception& e){ e; }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_FRAME Audio::GetFrameFromFilter(int i_)
{
	SHP_FRAME frame = make_shared<FRAME>();
	av_buffersink_get_frame(filter->data.sinkVec[i_], frame->Get());
	return frame;
}
