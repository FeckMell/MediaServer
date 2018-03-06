#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "Functions.h"
#include "Structs.h"
extern Logger* CLogger;
extern string DateStr;

class CConfPoint
{
public:
	CConfPoint(string SDPff, string SDPfc, string CallID, int port, asio::io_service& io_service);

	string ModifyPoint(string SDPff);
	void free();

	string CallID_; // CALLID ������
	int my_port_; // ����������� ����� ������ ����
	int remote_port_; // ���� �������
	string remote_ip_;// ip �������
	string SDP_; // sdp ��� ffmpeg
	bool mode = false; // active, inactive (hold)
	int error;

	Data RawBuf;
	CThreadedCircular FrameBuf;
	RTP_struct rtp;

	AVFormatContext* out_ifcx;
	AVCodecContext* out_iccx;
	AVFormatContext* ifcx;
	AVCodecContext* iccx;

	SHP_Socket Sock;
	udp::endpoint Endpoint;


	asio::io_service& io_service_;
private:
	void loggit(string a);

	void ChangeMode(string SDP);
	string ChangeVersion(string SDP);
	void ModifySDP();

	int open_output();
	int open_input();
	int sdp_open(AVFormatContext **pctx, const char *data, AVDictionary **options);

//	string SDP_; // sdp ��� ffmpeg
	string SDP_for_client;

	bool need_free = false;
};

typedef std::shared_ptr<CConfPoint> SHP_CConfPoint;