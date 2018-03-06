#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "CnfPoint.h"
#include "Filter.h"
using namespace std;

extern SHP_IPar init_Params;



//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
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
	SHP_CAVFrame Decode(SHP_CAVPacket, int);
	void ProceedData(int);
	void FillFilter(int);
	void EncodeAndSend(SHP_CAVFrame, int);
	SHP_CAVFrame GetFrameFromFilter(int);

	/*Data*/
	SHP_CAVFrame silentFrame;
	SHP_thread eventThread;
	bool state;// = true;
	Data rawBuf;
	SHP_Filter filter;
	vector<SHP_CnfPoint> vecPoints;
};
typedef shared_ptr<Audio> SHP_Audio;