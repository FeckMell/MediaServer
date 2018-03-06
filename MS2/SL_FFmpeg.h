#pragma once
#include "stdafx.h"

class PACKET
{
public:

	PACKET();
	PACKET(uint8_t* data_, unsigned size_);
	~PACKET();

	AVPacket* Get();
	uint8_t* Data();
	unsigned Size();

private:

	AVPacket _packet;

};
typedef shared_ptr<PACKET> SHP_PACKET;

class FRAME
{
public:

	FRAME();
	~FRAME();

	AVFrame* Get();

private:

	AVFrame _frame;

};
typedef shared_ptr<FRAME> SHP_FRAME;

class FFF
{
public:

	~FFF();

	vector<vector<AVFilterContext*>> _afcx;
	vector<AVFilterGraph*> _graphVec;
	vector<AVFilterContext*> _sinkVec;
	
	static SHP_PACKET SumPackets(SHP_PACKET p1_, SHP_PACKET p2_);

};