#pragma once
#include "../../stdafx.h"

class SockBuf; typedef shared_ptr<SockBuf> SHP_SockBuf; typedef shared_ptr<vector<SHP_SockBuf>> SHP_VecSockBuf;

class SockBuf
{
public:

	SockBuf(uint8_t* data_, int size_);

	uint8_t* Data();
	int Size();

	void AddFront(uint8_t* data_, int size_);
	void AddBack(uint8_t* data_, int size_);

	pair<SHP_SockBuf, SHP_SockBuf> Break(int first_size_);
	static vector<SHP_SockBuf> CutData(uint8_t* data_, int full_size_, int chunk_size_);

private:

	uint8_t data[2048];
	int size = 0;

}; 