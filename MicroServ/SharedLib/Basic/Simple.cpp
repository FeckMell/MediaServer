#include "stdafx.h"
#include "Simple.h"


SockBuf::SockBuf(uint8_t* data_, int size_) :size(size_)
{
	memcpy(data, data_, size_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
uint8_t* SockBuf::Data()
{
	return data;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int SockBuf::Size()
{
	return size;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SockBuf::AddFront(uint8_t* data_, int size_)
{
	if (size + size_ < 2047)
	{
		uint8_t new_data[2048];
		memcpy(new_data, data_, size_);
		memcpy(new_data + size_, data, size);
		size += size_;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SockBuf::AddBack(uint8_t* data_, int size_)
{
	if (size + size_ < 2047)
	{
		memcpy(data + size, data_, size_);
		size += size_;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
pair<SHP_SockBuf, SHP_SockBuf> SockBuf::Break(int first_size_)
{
	if (first_size_ > 0 && first_size_ < Size())
	{
		SHP_SockBuf first; first.reset(new SockBuf(Data(), first_size_));
		SHP_SockBuf second; second.reset(new SockBuf(Data() + first_size_, Size() - first_size_));
		return{ first, second };
	}
	else
	{
		SHP_SockBuf first; first.reset(new SockBuf(Data(), Size()));
		return{ first, nullptr };
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
vector<SHP_SockBuf> SockBuf::CutData(uint8_t* data_, int full_size_, int chunk_size_)
{
	vector<SHP_SockBuf> result;
	int last_index = full_size_ / chunk_size_;
	int last_chunk_size = full_size_ - last_index;
	for (int i = 0; i < last_index; ++i)
	{
		SHP_SockBuf new_chunk; new_chunk.reset(new SockBuf(data_ + i*chunk_size_, chunk_size_));
		result.push_back(new_chunk);
	}
	//if (last_chunk_size != 0)
	//{
	//	SHP_SockBuf last_chunk; last_chunk.reset(new SockBuf(data_ + last_index, last_chunk_size));
	//	result.push_back(last_chunk);
	//}
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------