#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "Functions.h"
#include "Structs.h"

#define INPUT_SAMPLERATE     8000
#define INPUT_FORMAT         AV_SAMPLE_FMT_S16
#define INPUT_CHANNEL_LAYOUT AV_CH_LAYOUT_STEREO
#define OUTPUT_BIT_RATE 8000
#define OUTPUT_CHANNELS 1
#define OUTPUT_SAMPLE_FORMAT AV_SAMPLE_FMT_S16
#define VOLUME_VAL 0.90

using namespace std;
using namespace std::chrono;
extern string DateStr;
extern Logger CLogger;

class CMixInit
{
public:
	CMixInit(){}
	CMixInit(vector<AVCodecContext*>iccx, vector<AVCodecContext*>out_iccx, int ID);

	Initing data;
	void FreeSockFFmpeg();
private:
	void loggit(string a);
	int init_filter_graph(int ForClient);

	int tracks = 0;

	vector<AVCodecContext*>iccx_;
	vector<AVCodecContext*>out_iccx_;

	int ID_;
};
typedef std::shared_ptr<CMixInit> SHP_CMixInit;