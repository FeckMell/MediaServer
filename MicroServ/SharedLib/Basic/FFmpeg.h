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

private:

	AVPacket packet;

};
typedef shared_ptr<PACKET> SHP_PACKET;
