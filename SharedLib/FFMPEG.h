#pragma once
#include "stdafx.h"

class PACKET
{
public:
	PACKET();
	PACKET(size_t sz);
	~PACKET();

	AVPacket* Get();
	int Size();
	uint8_t* Data();
	void Free();
	void MakeSize(int n);
private:
	AVPacket packet;
};
typedef shared_ptr<PACKET> SHP_PACKET;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class FRAME
{
public:
	FRAME();
	FRAME(bool a);
	~FRAME();

	AVFrame* Get();
	void Free();
	bool Empty();
private:
	bool empty;
	AVFrame* frame;
};
typedef shared_ptr<FRAME> SHP_FRAME;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
struct FFF
{
	~FFF()
	{
		for (auto &e : sinkVec) avfilter_free(e);//?
		for (auto &e1 : afcx) for (auto &e2 : e1) avfilter_free(e2);//??
		for (auto &e : graphVec) avfilter_graph_free(&e);//!
	}
	std::vector<vector<AVFilterContext*>> afcx;
	std::vector<AVFilterGraph*> graphVec;
	std::vector<AVFilterContext*> sinkVec;
};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------