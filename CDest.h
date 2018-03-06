#include "SrcCash.h"
//#include "stdafx.h"
using namespace std;

struct TRTP_Dest
{
	string	 strAddr;
	uint16_t portDest;
	uint16_t portSrc;
	uint16_t ptimeRTP;
};

class CDest
{
public:
	CDest(const TRTP_Dest& rtpdest){ openRTP(rtpdest); }

	int openRTP(const TRTP_Dest& rtpdest);
private:
	AVFormatContext*	ctxFormat_ = nullptr;

};