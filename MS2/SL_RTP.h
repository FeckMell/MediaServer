#pragma once
#include "stdafx.h"

struct RTPHEADER
{
	/* первый байт */
	uint8_t _csrc_len : 4;//
	uint8_t _extension : 1;//
	uint8_t _padding : 1;//
	uint8_t _version : 2;//
	/* второй байт */
	uint8_t _payload_type : 7;//
	uint8_t _marker : 1;//
	/* третий-четвертый байты */
	uint16_t _seq_no;
	/* пятый-восьмой байты */
	uint32_t _timestamp;//
	/* девятый-двенадцатый байт */
	uint32_t _ssrc;//
};

class RTP
{
public:

	RTP();

	uint8_t* Get(int ptime_);
	string PrintAll();

private:

	RTPHEADER _header;

};