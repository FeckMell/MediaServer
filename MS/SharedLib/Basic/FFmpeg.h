#pragma once
#include "../../stdafx.h"

class PACKET
{
public:
	PACKET();
	PACKET(size_t size_);
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
	~FRAME();

	AVFrame* Get();
	void Free();

private:

	AVFrame* frame;
};
typedef shared_ptr<FRAME> SHP_FRAME;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
struct FFF
{
public:
	~FFF()
	{
		for (auto &e : sinkVec) avfilter_free(e);
		for (auto &e1 : afcx) for (auto &e2 : e1) avfilter_free(e2);
		for (auto &e : graphVec) avfilter_graph_free(&e);
	}
	std::vector<vector<AVFilterContext*>> afcx;
	std::vector<AVFilterGraph*> graphVec;
	std::vector<AVFilterContext*> sinkVec;

	static SHP_PACKET AddToPacket(SHP_PACKET, SHP_PACKET);
	static SHP_PACKET CreatePacket(uint8_t*, int);
};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------