//#ifdef WIN32
#include "stdafx.h"
//#endif
//#ifdef linux
//#include "stdinclude.h"
//#endif
#include "CFilterInit.h"
#include "Functions.h"

//-*/-----------------------------------------------------------------------------------------
//-*/-----------------------------------------------------------------------------------------
void CFilterInit::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	steady_clock::time_point t1 = steady_clock::now();
	string result = DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	result += " ID=" + to_string(ID_) + " thread=" + boost::to_string(this_thread::get_id()) + "      ";
	CLogger->AddToLog(4, "\n" + result + a); 
}
//-*/-----------------------------------------------------------------------------------------
//-*/-----------------------------------------------------------------------------------------
//-*/-----------------------------------------------------------------------------------------
//-*/-----------------------------------------------------------------------------------------
CFilterInit::CFilterInit(vector<SHP_CConfPoint>Caller, int ID) :Caller_(Caller), tracks(Caller.size()), ID_(ID)
{
	string log = "";
	for (auto &e : Caller_)
	{
		log += to_string(e->my_port_) + "->" + e->remote_ip_ + ":" + to_string(e->remote_port_)+"\n";
	}
	log += "\n";
	for (auto &e : Caller_)
	{
		log += e->SDP_ + "\n";
	}
	loggit("init for tracks=" + to_string(tracks)+"\n route:\n"+log);
	

	int err;
	data.afcx.resize(tracks);
	for (int i = 0; i < tracks; ++i)
		data.afcx[i].src.resize(tracks - 1);

	for (int i = 0; i < tracks; ++i)
	{
		err = init_filter_graph(i);
		if (err < 0)
		{
			loggit("Init err =  " + to_string(err) + "for i= " + to_string(i));
			system("pause");
		}
	}
	loggit("Mixer init DONE");
}
//-*/-----------------------------------------------------------------------------------------
//-*/-----------------------------------------------------------------------------------------
int CFilterInit::init_filter_graph(int ForClient)
{
	loggit("int ConfAudio::init_filter_graph");
	AVFilter        *mix_filter;
	AVFilterContext *mix_ctx;
	AVFilter        *abuffersink;
	AVFilterContext *abuffersink_ctx;
	AVFilterGraph   *filter_graph;
	char args[512];
	char arg[10];
	int err;

	/* Создаем фильтр-граф, содержащий все фильтры */
	filter_graph = avfilter_graph_alloc();
	if (!filter_graph)
	{
		loggit("Unable to create filter graph.");
		av_log(NULL, AV_LOG_ERROR, "Unable to create filter graph.\n");
		return AVERROR(ENOMEM);
	}

	/****** abuffer [ForClient][i] ********/
	for (int i = 0; i < tracks; ++i)
	{
		/*for i==ForClient we dont set buffer*/
		if (i == ForClient) continue;// вернуть
		AVFilter *abuffer0;
		/* Создаем абуффер фильтр. он используется для "скармливания" информации в граф */
		abuffer0 = avfilter_get_by_name("abuffer");
		if (!abuffer0)
		{
			//string s(get_error_text(error));
			loggit("Could not find the abuffer filter.");
			av_log(NULL, AV_LOG_ERROR, "Could not find the abuffer filter.\n");
			return AVERROR_FILTER_NOT_FOUND;
		}
		/*источник буффера: раскодированные фреймы из декодера будут в здесь*/
		/* buffer audio source: the decoded frames from the decoder will be inserted here. */
		if (!Caller_[i]->iccx->channel_layout)
		{
			Caller_[i]->iccx->channel_layout = av_get_default_channel_layout(Caller_[i]->iccx->channels);

		}
		snprintf(args, sizeof(args), "sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
			Caller_[i]->iccx->sample_rate, av_get_sample_fmt_name(Caller_[i]->iccx->sample_fmt), Caller_[i]->iccx->channel_layout);
		//snprintf(arg, sizeof(arg), "src%d-%d", ForClient, i);
		snprintf(arg, sizeof(arg), "src");
		//разбиение индекса для SSource.
		if (i < ForClient)
		{
			err = avfilter_graph_create_filter(&data.afcx[ForClient].src[i], abuffer0, arg, args, NULL, filter_graph);
		}
		else
		{
			err = avfilter_graph_create_filter(&data.afcx[ForClient].src[i - 1], abuffer0, arg, args, NULL, filter_graph);
		}

		if (err < 0)
		{
			loggit("Cannot create audio buffer source");
			av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer source\n");
			return err;
		}
	}

	/****** amix ******* */
	/* Create mix filter. */
	mix_filter = avfilter_get_by_name("amix");
	if (!mix_filter)
	{
		loggit("Could not find the mix filter.");
		av_log(NULL, AV_LOG_ERROR, "Could not find the mix filter.\n");
		return AVERROR_FILTER_NOT_FOUND;
	}

	//snprintf(arg, sizeof(arg), "amix%d", ForClient);
	snprintf(arg, sizeof(arg), "amix");
	snprintf(args, sizeof(args), "inputs=%d", tracks - 1);

	err = avfilter_graph_create_filter(&mix_ctx, mix_filter, arg,
		args, NULL, filter_graph);

	if (err < 0)
	{
		loggit("Cannot create audio amix filter");
		av_log(NULL, AV_LOG_ERROR, "Cannot create audio amix filter\n");
		return err;
	}

	/* Finally create the abuffersink filter;* it will be used to get the filtered data out of the graph. */
	abuffersink = avfilter_get_by_name("abuffersink");
	if (!abuffersink)
	{
		loggit("Could not find the abuffersink filter.");
		av_log(NULL, AV_LOG_ERROR, "Could not find the abuffersink filter.\n");
		return AVERROR_FILTER_NOT_FOUND;
	}

	//snprintf(arg, sizeof(arg), "sink%d", ForClient);
	snprintf(arg, sizeof(arg), "sink");
	abuffersink_ctx = avfilter_graph_alloc_filter(filter_graph, abuffersink, arg);
	if (!abuffersink_ctx)
	{
		loggit("Could not allocate the abuffersink instance.");
		av_log(NULL, AV_LOG_ERROR, "Could not allocate the abuffersink instance.\n");
		return AVERROR(ENOMEM);
	}

	/* Same sample fmts as the output file. */
	const enum AVSampleFormat Fmts[] = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
	err = av_opt_set_int_list(abuffersink_ctx, "sample_fmts", Fmts, AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN);

	char ch_layout[64];
	av_get_channel_layout_string(ch_layout, sizeof(ch_layout), 0, OUTPUT_CHANNELS);
	av_opt_set(abuffersink_ctx, "channel_layout", ch_layout, AV_OPT_SEARCH_CHILDREN);

	if (err < 0)
	{
		loggit("Could set options to the abuffersink instance." + err);
		av_log(NULL, AV_LOG_ERROR, "Could set options to the abuffersink instance.\n");
		return err;
	}

	err = avfilter_init_str(abuffersink_ctx, NULL);
	if (err < 0)
	{
		loggit("Could not initialize the abuffersink instance." + err);
		av_log(NULL, AV_LOG_ERROR, "Could not initialize the abuffersink instance.\n");
		return err;
	}

	/* Connect the filters; */
	int indexx = 0;
	for (int i = 0; i < tracks - 1; ++i)
	{
		//if (i == ForClient) continue;
		err = avfilter_link(data.afcx[ForClient].src[i], 0, mix_ctx, i);
		if (err < 0)
		{
			loggit("Error connecting filters " + to_string(i));
			av_log(NULL, AV_LOG_ERROR, "Error connecting filters\n");
			return err;
		}
		++indexx;
	}
	err = avfilter_link(mix_ctx, 0, abuffersink_ctx, 0);
	if (err < 0)
	{
		loggit("Error connecting filters" + to_string(err));
		av_log(NULL, AV_LOG_ERROR, "Error connecting filters\n");
		return err;
	}


	/* Configure the graph. */
	err = avfilter_graph_config(filter_graph, NULL);
	if (err < 0)
	{
		string s(get_error_text(err));
		loggit("Error while configuring graph :" + s);
		av_log(NULL, AV_LOG_ERROR, "Error while configuring graph : %s\n", get_error_text(err));
		return err;
	}

	char* dump = avfilter_graph_dump(filter_graph, NULL);
	av_log(NULL, AV_LOG_ERROR, "Graph :\n%s\n", dump);

	data.graphVec.push_back(filter_graph);
	data.sinkVec.push_back(abuffersink_ctx);

	loggit("int ConfAudio::init_filter_graph END");
	return 0;
}
//-*/-----------------------------------------------------------------------------------------
//-*/-----------------------------------------------------------------------------------------
void CFilterInit::FreeSockFFmpeg()
{
	loggit("Destroing filter");//
	for (auto &e : data.graphVec)
	{
		avfilter_graph_free(&e);
	}

	//for (int i = 0; i < tracks; ++i)
	//{
		//avcodec_close(data.out_iccx[i]);//
		//avformat_close_input(&data.ifcx[i]);
		//data.ifcx[i] = NULL;
		
		//avcodec_free_context(&data.out_iccx[i]);
		//avio_close(data.out_ifcx[i]->pb);
		//avformat_free_context(data.ifcx[i]);
		

		//---------------------------------------------------------------
		//avformat_free_context(data.out_ifcx[i]);
		//avcodec_close(iccx_[i]);
		//avcodec_close(data.out_iccx[i]);
		//avcodec_free_context(&iccx_[i]);
	//}
	loggit("Destroing filter DONE!");//
}
//-*/-----------------------------------------------------------------------------------------
//-*/-----------------------------------------------------------------------------------------

