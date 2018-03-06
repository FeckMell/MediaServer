#include "stdafx.h"
#include "Filter.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Filter::Filter(vector<SHP_CnfPoint> points_) : cnfPoints(points_)
{
	tracks = cnfPoints.size();
	data.afcx.resize(tracks);
	for (int i = 0; i < tracks; ++i)
		data.afcx[i].resize(tracks - 1);

	for (int i = 0; i < tracks; ++i)
	{
		if (InitFilterGraph(i) < 0)
		{
			cout << "\nFilter::Filter init error";
			system("pause");
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int Filter::InitFilterGraph(int for_client_)
{
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

	/****** abuffer [for_client_][i] ********/
	for (int i = 0; i < tracks; ++i)
	{
		/*for i==for_client_ we dont set buffer*/
		if (i == for_client_) continue;// вернуть
		AVFilter *abuffer0;
		/* Создаем абуффер фильтр. он используется для "скармливания" информации в граф */
		abuffer0 = avfilter_get_by_name("abuffer");
		if (!abuffer0)
		{
			return AVERROR_FILTER_NOT_FOUND;
		}
		/*источник буффера: раскодированные фреймы из декодера будут в здесь*/
		/* buffer audio source: the decoded frames from the decoder will be inserted here. */
		if (!cnfPoints[i]->iccx->channel_layout)
		{
			cnfPoints[i]->iccx->channel_layout = av_get_default_channel_layout(cnfPoints[i]->iccx->channels);
		}
		snprintf(
			args, 
			sizeof(args),
			"sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
			cnfPoints[i]->iccx->sample_rate,
			av_get_sample_fmt_name(cnfPoints[i]->iccx->sample_fmt),
			cnfPoints[i]->iccx->channel_layout
			);
		//snprintf(arg, sizeof(arg), "src%d-%d", for_client_, i);
		snprintf(arg, sizeof(arg), "src");
		//разбиение индекса для SSource.
		if (i < for_client_)
		{
			err = avfilter_graph_create_filter(&data.afcx[for_client_][i], abuffer0, arg, args, NULL, filter_graph);
		}
		else
		{
			err = avfilter_graph_create_filter(&data.afcx[for_client_][i - 1], abuffer0, arg, args, NULL, filter_graph);
		}

		if (err < 0) { return err; }
	}

	/****** amix ******* */
	/* Create mix filter. */
	mix_filter = avfilter_get_by_name("amix");
	if (!mix_filter)
	{
		return AVERROR_FILTER_NOT_FOUND;
	}

	//snprintf(arg, sizeof(arg), "amix%d", for_client_);
	snprintf(arg, sizeof(arg), "amix");
	snprintf(args, sizeof(args), "inputs=%d", tracks - 1);

	err = avfilter_graph_create_filter(&mix_ctx, mix_filter, arg, args, NULL, filter_graph);
	if (err < 0) { return err; }

	/* Finally create the abuffersink filter;* it will be used to get the filtered data out of the graph. */
	abuffersink = avfilter_get_by_name("abuffersink");
	if (!abuffersink){ return AVERROR_FILTER_NOT_FOUND; }

	//snprintf(arg, sizeof(arg), "sink%d", for_client_);
	snprintf(arg, sizeof(arg), "sink");
	abuffersink_ctx = avfilter_graph_alloc_filter(filter_graph, abuffersink, arg);
	if (!abuffersink_ctx){ return AVERROR(ENOMEM); }

	/* Same sample fmts as the output file. */
	const enum AVSampleFormat Fmts[] = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
	err = av_opt_set_int_list(abuffersink_ctx, "sample_fmts", Fmts, AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN);

	char ch_layout[64];
	av_get_channel_layout_string(ch_layout, sizeof(ch_layout), 0, OUTPUT_CHANNELS);
	av_opt_set(abuffersink_ctx, "channel_layout", ch_layout, AV_OPT_SEARCH_CHILDREN);
	if (err < 0){ return err; }

	err = avfilter_init_str(abuffersink_ctx, NULL);
	if (err < 0){ return err; }

	/* Connect the filters; */
	int indexx = 0;
	for (int i = 0; i < tracks - 1; ++i)
	{
		//if (i == for_client_) continue;
		err = avfilter_link(data.afcx[for_client_][i], 0, mix_ctx, i);
		if (err < 0){ return err; }
		++indexx;
	}
	err = avfilter_link(mix_ctx, 0, abuffersink_ctx, 0);
	if (err < 0){ return err; }

	/* Configure the graph. */
	err = avfilter_graph_config(filter_graph, NULL);
	if (err < 0){ return err; }

	char* dump = avfilter_graph_dump(filter_graph, NULL);
	av_log(NULL, AV_LOG_ERROR, "Graph :\n%s\n", dump);

	data.graphVec.push_back(filter_graph);
	data.sinkVec.push_back(abuffersink_ctx);

	return 0;
}