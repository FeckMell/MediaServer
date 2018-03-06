#pragma once
#include "stdafx.h"


struct REQUEST
{
	EP sender;
	char data[2048];
};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
struct RTP
{
	/* ������ ���� */
	uint8_t csrc_len : 4;//
	uint8_t extension : 1;//
	uint8_t padding : 1;//
	uint8_t version : 2;//
	/* ������ ���� */
	uint8_t payload_type : 7;//
	uint8_t marker : 1;//
	/* ������-��������� ����� */
	uint16_t seq_no;
	/* �����-������� ����� */
	uint32_t timestamp;//
	/* �������-����������� ���� */
	uint32_t ssrc;//
};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
struct RTP_struct
{
	RTP header;
	long TS;
	int amount;

	RTP_struct();
	void Config();
	RTP Get();
};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class SOCK
{
public:
	SOCK(string, int, SHP_IO);
	~SOCK();

	void ChangeIO(SHP_IO);
	void AsyncReceive(boost::function<void(boost::system::error_code, size_t)> boostbind_);
	void SendTo(uint8_t*, int);
	void SetEndPoint(string, string);
	uint8_t* GetRTP();

	EP endPoint;
	uint8_t buffer[2048];
	SHP_IO io;
	boost::asio::ip::udp::socket s;

private:

	RTP_struct rtp;

};
typedef shared_ptr<SOCK> SHP_SOCK;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------