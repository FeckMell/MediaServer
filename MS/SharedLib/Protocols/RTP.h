#pragma once
#include "../../stdafx.h"

struct RTPBase
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
class RTP
{
public:

	RTP();
	uint8_t* Get(int ptime_);

private:

	RTPBase header;
	long TS;
	int amount;


};