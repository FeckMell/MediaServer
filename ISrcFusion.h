#pragma once
#include "envlp_ffmpeg.h"

class ISrcFusion
{
public:
	bool operator ==(const ISrcFusion &rhv) const //перегрузка оператора сравнения
	{
		return Name() == rhv.Name();
	}
	virtual const AVCodecContext *CodecCTX()const = 0;
	virtual SHP_CScopedPFrame getNextDecoded(bool& bEOF) = 0;
	virtual const string& Name() const = 0;
};
typedef shared_ptr<ISrcFusion> SHP_ISrcFusion;