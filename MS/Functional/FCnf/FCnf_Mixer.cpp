#include "stdafx.h"
#include "FCnf_Mixer.h"
using namespace NFCnf;

Mixer::Mixer(vector<SHP_Point> vec_point_) : vecPoint(vec_point_)
{
	cnfIO.reset(new IO());
	shpFilter.reset(new Filter(vecPoint));
	CreateSilentFrame();

		fakeSocket.reset(new Socket(CFG::Param("MyFakeIP"), CFG::Param("MyFakePort"), cnfIO));
		fakeSocket->SetEndPoint(CFG::Param("OuterFakeIP"), CFG::Param("OuterFakePort"));
		fakeSocket->AsyncReceiveFrom(boost::bind(&Mixer::FakeReceive, this, _1, _2));
		actionThread.reset(new thread([&](){cnfIO->run(); }));

	
	for (int i = 0; i < (int)vecPoint.size(); ++i)
	{
		vecPoint[i]->baseCaller->AddReceiveFunc("cnf", boost::bind(&Mixer::PointReceiveAction, this, i, _1));
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Mixer::~Mixer()
{
	
}
void Mixer::StopActivity()
{
	for (auto& e : vecPoint) e->baseCaller->RemReceiveFunc("cnf");
	fakeSocket->Cancel();
	actionThread->join();
	fakeSocket.reset();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Mixer::CreateSilentFrame()
{
	int mark;
	silentFrame; silentFrame.reset(new FRAME());
	SHP_PACKET shpPacket; shpPacket.reset(new PACKET(80));
	string str(80, 1);
	memcpy(shpPacket->Data(), str.c_str(), 80);
	avcodec_decode_audio4(vecPoint[0]->iccx, silentFrame->Get(), &mark, shpPacket->Get());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Mixer::PointReceiveAction(int i_, SHP_SockBuf sock_buf_)
{
	cnfIO->post(boost::bind(&Mixer::ProceedData, this, i_, sock_buf_));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Mixer::ProceedData(int i_, SHP_SockBuf sock_buf_)
{
	/*Copy data and set ptime*/
	vecPoint[i_]->ptime = (sock_buf_->Size() - 12) / 80;
	SHP_PACKET shpPacket = FFF::CreatePacket(sock_buf_->Data() + 12, sock_buf_->Size() - 12);

	/*Split to ptime=10 packets, decode and store in jitter*/
	for (int j = 0; j < vecPoint[i_]->ptime; ++j)
	{
		SHP_PACKET temp_packet = FFF::CreatePacket(shpPacket->Data() + j * 80, 80);
		SHP_FRAME temp_frame = Decode(temp_packet, i_);
		SaveFrameToJitters(temp_frame, i_);
	}

	/* Fill this point filter with frames from others*/
	for (int k = 0; k < vecPoint[i_]->ptime; ++k) FillFilter(i_);

	/* Get filtered frame, encode, send */
	EncodeAndSend(i_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_FRAME Mixer::Decode(SHP_PACKET packet_, int i_)
{
	SHP_FRAME frame; frame.reset(new FRAME());
	int mark;
	avcodec_decode_audio4(vecPoint[i_]->iccx, frame->Get(), &mark, packet_->Get());
	return frame;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Mixer::SaveFrameToJitters(SHP_FRAME frame_, int from_which_)
{
	for (int i = 0; i < (int)vecPoint.size(); ++i)
	{
		if (i == from_which_) continue;

		vecPoint[i]->StoreFrame(frame_, from_which_);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Mixer::FillFilter(int i_)
{
	for (int j = 0; j < (int)vecPoint.size(); ++j)// from all other buffers
	{
		if (i_ == j) continue;

		SHP_FRAME frame = vecPoint[i_]->GetFrame(j);

		if (frame == nullptr) { frame = silentFrame; }
		if (i_ < j){ av_buffersrc_write_frame(shpFilter->data.afcx[i_][j - 1], frame->Get()); }
		else{ av_buffersrc_write_frame(shpFilter->data.afcx[i_][j], frame->Get()); }
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Mixer::EncodeAndSend(int i_)
{
	SHP_SockBuf result; result.reset(new SockBuf());

	for (int j = 0; j < vecPoint[i_]->ptime; ++j)
	{
		int mark;
		SHP_PACKET temp_packet; temp_packet.reset(new PACKET());
		SHP_FRAME temp_frame; temp_frame.reset(new FRAME());

		av_buffersink_get_frame(shpFilter->data.sinkVec[i_], temp_frame->Get());
		avcodec_encode_audio2(vecPoint[i_]->occx, temp_packet->Get(), temp_frame->Get(), &mark);
		result->AddData(temp_packet->Data(), temp_packet->Size());
	}

	vecPoint[i_]->baseCaller->SendTo(result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Mixer::FakeReceive(boost::system::error_code ec_, size_t size_)
{
	if (!ec_)
	{
		LOG::Log("fatal", "Errors", "Mixer::FakeReceive");
		this_thread::sleep_for(chrono::milliseconds(5)); //DebugTest
		fakeSocket->AsyncReceiveFrom(boost::bind(&Mixer::FakeReceive, this, _1, _2));
	}
}