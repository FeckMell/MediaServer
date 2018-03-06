#pragma once
#include "../SharedSource/stdafx.h"
#include "../SharedSource/Structs.h"
#include "../SharedSource/Functions.h"
#include "CnfPoint.h"
#include "Filter.h"

extern SHP_STARTUP init_Params;

class Audio
{
public:
	Audio(vector<SHP_CnfPoint>);
	void MD(vector<SHP_CnfPoint>);
	void Stop();

private:
	/*initing and reiniting*/
	void CreateSilentFrame();
	void Run();
	void RunIO();
	
	/*Main activity*/
	void Receive(boost::system::error_code, size_t, int);
	SHP_FRAME Decode(SHP_PACKET, int);
	void ProceedData(int);
	void FillFilter(int);
	void EncodeAndSend(SHP_FRAME, int);
	SHP_FRAME GetFrameFromFilter(int);

	/*Data*/
	SHP_FRAME silentFrame;
	SHP_thread eventThread;
	bool state;// = true;
	Data rawBuf;
	SHP_Filter filter;
	vector<SHP_CnfPoint> vecPoints;
};
typedef shared_ptr<Audio> SHP_Audio;