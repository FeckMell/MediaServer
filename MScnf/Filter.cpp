#include "stdafx.h"
#include "Filter.h"
using namespace cnf;


//TODO: logs

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
			BOOST_LOG_SEV(LOG::vecLogs, fatal) << "Filter::Filter(..) ERROR";
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

	auto template_arg_create_graph = boost::format(string("sample_rate=%1%:sample_fmt=%2%:channel_layout=0x%3%"));
	auto template_arg_create_filter = boost::format(string("inputs=%1%"));

	int err;

	/* Создаем фильтр-граф, содержащий все фильтры */
	filter_graph = avfilter_graph_alloc();

	/****** abuffer [for_client_][i] ********/
	for (int i = 0; i < tracks; ++i)
	{
		/*for i==for_client_ we dont set buffer*/
		if (i == for_client_) continue;
		
		/* Создаем абуффер фильтр. он используется для "скармливания" информации в граф */
		AVFilter *abuffer0;
		abuffer0 = avfilter_get_by_name("abuffer");
		if (!abuffer0){ return AVERROR_FILTER_NOT_FOUND; }

		/*источник буффера: раскодированные фреймы из декодера будут в здесь*/
		if (!cnfPoints[i]->iccx->channel_layout)
			{ cnfPoints[i]->iccx->channel_layout = av_get_default_channel_layout(cnfPoints[i]->iccx->channels); }
		
		string arg1 = str(template_arg_create_graph
			%cnfPoints[i]->iccx->sample_rate
			%av_get_sample_fmt_name(cnfPoints[i]->iccx->sample_fmt)
			%cnfPoints[i]->iccx->channel_layout
			);
		//разбиение индекса для SSource.
		if (i < for_client_) { err = avfilter_graph_create_filter(&data.afcx[for_client_][i], abuffer0, "src", arg1.c_str(), NULL, filter_graph); }
		else { err = avfilter_graph_create_filter(&data.afcx[for_client_][i - 1], abuffer0, "src", arg1.c_str(), NULL, filter_graph); }

		if (err < 0) { return err; }
	}

	/****** amix ******* */
	/* Create mix filter. */
	mix_filter = avfilter_get_by_name("amix");
	if (!mix_filter){ return AVERROR_FILTER_NOT_FOUND; }

	string arg2 = str(template_arg_create_filter % (tracks - 1));
	err = avfilter_graph_create_filter(&mix_ctx, mix_filter, "amix", arg2.c_str(), NULL, filter_graph);
	if (err < 0) { return err; }

	/* Finally create the abuffersink filter;* it will be used to get the filtered data out of the graph. */
	abuffersink = avfilter_get_by_name("abuffersink");
	if (!abuffersink){ return AVERROR_FILTER_NOT_FOUND; }

	abuffersink_ctx = avfilter_graph_alloc_filter(filter_graph, abuffersink, "sink");
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

	data.graphVec.push_back(filter_graph);
	data.sinkVec.push_back(abuffersink_ctx);

	return 0;
}