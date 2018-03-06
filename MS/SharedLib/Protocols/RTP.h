#pragma once
#include "../../stdafx.h"

struct RTPBase
{
	/* первый байт */
	uint8_t csrc_len : 4;//
	uint8_t extension : 1;//
	uint8_t padding : 1;//
	uint8_t version : 2;//
	/* второй байт */
	uint8_t payload_type : 7;//
	uint8_t marker : 1;//
	/* третий-четвертый байты */
	uint16_t seq_no;
	/* пятый-восьмой байты */
	uint32_t timestamp;//
	/* девятый-двенадцатый байт */
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