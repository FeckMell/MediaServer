#pragma once
#include "SL_All.h"

class Receiver
{
public:

	Receiver();

private:

	void Run();
	void ReceiveSIP(boost::system::error_code, size_t);
	void ReceiveMGCP(boost::system::error_code, size_t);
};
