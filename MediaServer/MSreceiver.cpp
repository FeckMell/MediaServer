#include "stdafx.h"
#include "MSreceiver.h"

Receiver::Receiver()
{
	Run();
	NET::outerIO->run();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Receiver::Run()
{
	if (CFG::data["sipName"] != "") NET::GS(NET::OUTER::sip_)->AsyncReceive(boost::bind(&Receiver::ReceiveSIP, this, _1, _2));
	NET::GS(NET::OUTER::mgcp_)->AsyncReceive(boost::bind(&Receiver::ReceiveMGCP, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Receiver::ReceiveSIP(boost::system::error_code ec_, size_t size_)
{
	if (size_ > 10) 
	{ 
		NET::GS(NET::OUTER::sip_)->buffer[size_] = 0;
		NET::vecSigsOUT[NET::OUTER::sip_]();
	}
	NET::GS(NET::OUTER::sip_)->AsyncReceive(boost::bind(&Receiver::ReceiveSIP, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Receiver::ReceiveMGCP(boost::system::error_code ec_, size_t size_)
{
	if (size_ > 10)
	{
		NET::GS(NET::OUTER::mgcp_)->buffer[size_] = 0;
		NET::vecSigsOUT[NET::OUTER::mgcp_]();
	}
	NET::GS(NET::OUTER::mgcp_)->AsyncReceive(boost::bind(&Receiver::ReceiveMGCP, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
