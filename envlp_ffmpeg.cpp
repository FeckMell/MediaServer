#pragma once
#include "stdafx.h"
#include "envlp_ffmpeg.h"

#pragma comment (lib,"avformat.lib")
#pragma comment (lib,"avcodec.lib")
#pragma comment (lib,"avformat.lib")
#pragma comment (lib,"avutil.lib")
#pragma comment (lib,"avfilter.lib")
//#pragma comment (lib,"swresample.lib")

//-----------------------------------------------------------------------
const AVFilter* gl_abuffer;
const AVFilter* gl_abuffersink;
const AVFilter* gl_amix;

static class ffmpegInitialization
{
public:
	ffmpegInitialization()
	{
		av_register_all();
		avcodec_register_all();
		avformat_network_init();
		avfilter_register_all();

#ifdef _DEBUG
		av_log_set_level(AV_LOG_VERBOSE);
#else
		av_log_set_level(AV_LOG_ERROR);
#endif // _DEBUG

		gl_abuffer = avfilter_get_by_name("abuffer");
		gl_abuffersink = avfilter_get_by_name("abuffersink");
		gl_amix = avfilter_get_by_name("amix");

	}
}__Initialization;
