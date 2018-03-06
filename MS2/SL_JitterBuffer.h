#pragma once
#include "stdafx.h"
#include "SL_FFmpeg.h"

class SUBJITTER
{
public:

	SUBJITTER();

	void Add(SHP_FRAME);
	SHP_FRAME Get();

private:

	boost::circular_buffer<SHP_FRAME> _data;
};

class JITTER
{
public:

	JITTER();

	void Add(SHP_FRAME frame_, int where_);
	SHP_FRAME Get(int where_);

	void Resize(int size_);

private:

	vector<SUBJITTER> _buffers;

};