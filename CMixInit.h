#pragma once
#include "stdafx.h"
#include "Logger.h"
#include "Functions.h"
#include "Structs.h"
#include "ConfPoint.h"

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
	//CMixInit(vector<AVCodecContext*>iccx, vector<AVCodecContext*>out_iccx, int ID);
	CMixInit(vector<SHP_CConfPoint>Callers, int ID);



	Initing data;
	void FreeSockFFmpeg();
private:
	void loggit(string a);
	int init_filter_graph(int ForClient);


	int sdp_open(AVFormatContext **pctx, const char *data, AVDictionary **options);
	int open_input_file(int i);
	int open_output_file(int i);





	int tracks = 0;

	//vector<AVCodecContext*> iccx_;
	//vector<AVCodecContext*> out_iccx_;
	vector<SHP_CConfPoint> Callers;

	int ID_;
};
typedef std::shared_ptr<CMixInit> SHP_CMixInit;