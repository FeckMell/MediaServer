#include "stdafx.h"
#include "MSreceiver.h"

Receiver::Receiver()
{
	Run();
	COM::outerIO->run();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Receiver::Run()
{
	if (CFG::data["sipName"] != "") COM::GS(COM::OUTER::sip_)->AsyncReceive(boost::bind(&Receiver::ReceiveSIP, this, _1, _2));
	COM::GS(COM::OUTER::mgcp_)->AsyncReceive(boost::bind(&Receiver::ReceiveMGCP, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Receiver::ReceiveSIP(boost::system::error_code ec_, size_t size_)
{
	if (size_ > 10)
	{
		COM::GS(COM::OUTER::sip_)->buffer[size_] = 0;
		COM::vecSigsOUT[COM::OUTER::sip_]();
	}
	COM::GS(COM::OUTER::sip_)->AsyncReceive(boost::bind(&Receiver::ReceiveSIP, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Receiver::ReceiveMGCP(boost::system::error_code ec_, size_t size_)
{
	if (size_ > 10)
	{
		COM::GS(COM::OUTER::mgcp_)->buffer[size_] = 0;
		COM::vecSigsOUT[COM::OUTER::mgcp_]();
	}
	COM::GS(COM::OUTER::mgcp_)->AsyncReceive(boost::bind(&Receiver::ReceiveMGCP, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
