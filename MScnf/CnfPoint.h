#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
using namespace std;

extern SHP_IPar init_Params;


class CnfPoint
{
public:
	CnfPoint(string, string, string, string, boost::asio::io_service&);
	~CnfPoint();

	void SetMaxTimesTook(int);
	SHP_CAVFrame GetFrame();
	void StoreFrame(SHP_CAVFrame);

	string clientPort;
	string clientIP;
	string serverPort; //ID of point
	string serverIP;
	AVCodecContext* iccx;
	AVCodecContext* occx;
	SHP_Socket socket;
	RTP_struct rtp;
	udp::endpoint endPoint;
	IO& ioCnf;

private:
	void InitCodec(AVCodecContext**, bool);//true-decoder, false-encoder

	int timesTookMax = 0;
	int timesTook = 0;
	SHP_CAVFrame bufFrame = nullptr;
};
typedef shared_ptr<CnfPoint> SHP_CnfPoint;