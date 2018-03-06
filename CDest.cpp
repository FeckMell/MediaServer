#include "CDest.h"
#include "stdafx.h"

int CDest::openRTP(const TRTP_Dest& rtpdest)
{
	//isRTP_ = true;
	//ptimeRTP_ = rtpdest.ptimeRTP;

	const auto strRTP = str(boost::format("rtp://%1%:%2%?localport=%3%")
		% rtpdest.strAddr % rtpdest.portDest %  rtpdest.portSrc);
	avformat_open_input(&ctxFormat_, strRTP.c_str(), /*ifmt*/nullptr, nullptr);
	return 0;
}