#pragma once
#include "stdafx.h"

class PACKET
{
public:

	PACKET();
	PACKET(size_t sz);
	PACKET(uint8_t*, int);
	PACKET(PACKET*, PACKET*);
	~PACKET();

	AVPacket* Get();
	uint8_t* Data();
	int Size();
	void Add(AVPacket*);

private:

	void Free();
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

	//static SHP_PACKET AddToPacket(SHP_PACKET, SHP_PACKET);
	//static SHP_PACKET CreatePacket(uint8_t*, int);
};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class subjitter
{
public:
	subjitter();

	void Push(SHP_FRAME);
	SHP_FRAME Pop();

private:

	boost::circular_buffer<SHP_FRAME> frames;

};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class JITTER
{
public:
	JITTER();

	void Resize(int);
	void Push(SHP_FRAME, int);
	SHP_FRAME Pop(int);

private:

	vector<subjitter> buffers;

};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------