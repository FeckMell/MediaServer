#pragma once
#include "../../stdafx.h"
#include "FFmpeg.h"

class SubJitter
{
public:
	SubJitter();

	void Push(SHP_FRAME frame_);
	SHP_FRAME Pop();

private:

	boost::circular_buffer<SHP_FRAME> cirFrame;

};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class Jitter
{
public:
	Jitter();

	void Resize(int size_);
	void Push(SHP_FRAME frame_, int where_);
	SHP_FRAME Pop(int which_);

private:

	vector<SubJitter> vecSubJitter;

};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------