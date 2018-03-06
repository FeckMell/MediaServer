#include "stdafx.h"
#include "SL_BasicStructs.h"


RTP_struct::RTP_struct()
{
	Config();
}
//*///------------------------------------------------------------------------------------------
void RTP_struct::Config()
{
	header.version = 2;
	header.marker = 0;
	header.csrc_len = 0;
	header.extension = 0;
	header.padding = 0;
	header.ssrc = htons(10);
	header.payload_type = 8;
	header.timestamp = htonl(0);
	header.seq_no = htons(0);
}
//*///------------------------------------------------------------------------------------------
RTP* RTP_struct::Get(int ptime_)
{
	++amount;
	header.seq_no = htons(amount);
	header.timestamp = htonl(80 * ptime_ * amount);

	return &header;
}
//*///------------------------------------------------------------------------------------------
RTP* RTP_struct::Get()
{
	++amount;
	header.seq_no = htons(amount);
	header.timestamp = htonl(160 * amount);

	return &header;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------