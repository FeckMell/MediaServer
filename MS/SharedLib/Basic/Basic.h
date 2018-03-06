#pragma once
#include "../../stdafx.h"

class SockBuf
{
public:
	SockBuf();
	SockBuf(uint8_t* data_, int size_);
	void AddData(uint8_t* data_, int size_);

	int Size();
	uint8_t* Data();

private:

	uint8_t data[2048];
	int size;

};
typedef shared_ptr<SockBuf> SHP_SockBuf;