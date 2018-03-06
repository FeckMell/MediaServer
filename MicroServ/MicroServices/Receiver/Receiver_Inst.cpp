#include "stdafx.h"
#include "Receiver_Inst.h"
using namespace Receiver;

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Inst::Inst(SHP_IPL ipl_, SHP_IO io_)
{
	mapData["ModulName"] = ipl_->Param("ModulName");
	mapData["ServerIP"] = ipl_->Param("ServerIP");
	mapData["ServerPort"] = ipl_->Param("ServerPort");
	
	funcAction = ipl_->GetFunc("Func");

	socketThis.reset(new Socket("", "", Param("ServerIP"), Param("ServerPort"), io_));
	socketThis->AsyncReceiveFrom(boost::bind(&Inst::Receive, this, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Inst::Param(string name_)
{
	return mapData[name_];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Inst::Receive(boost::system::error_code ec_, size_t size_)
{
	if (!ec_)
	{
		SHP_SockBuf new_data; new_data.reset(new SockBuf(socketThis->Data(), size_));
		try
		{
			funcAction(pair<SHP_SockBuf, EP>(new_data, socketThis->GetEndPoint()));
		}
		catch (...)
		{
			LOG::Log("info2", "Receiver", "Receiver::Inst::Receive catch _" + mapData["ModulName"] + "_");
		}
		socketThis->ResetEndPoint();
		socketThis->AsyncReceiveFrom(boost::bind(&Inst::Receive, this, _1, _2));
	}
	else
	{
		LOG::Log("debug", "Receiver", "Receiver::Inst::Receive else");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------