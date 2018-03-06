#include "stdafx.h"
#include "FAnn_Point.h"
using namespace NFAnn;

Point::Point(SHP_IPL ipl_, SHP_MusicFile file_)
{
	mapData["From"] = ipl_->Param("From");
	mapData["Loop"] = ipl_->Param("Loop");
	mapData["FileName"] = ipl_->Param("FileName");
	mFile = file_;
	baseCaller = CallerStore::GetCaller(ipl_->Param("CallID"));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Point::~Point()
{

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::SendNextPacket()
{
	if ((Param("Loop") == "loop") && (fileProgress==(mFile->Size()-2)))
	{
		Report("Loop=done");
	}
	else
	{
		SHP_PACKET audio_packet = mFile->GetPacket(fileProgress);
		SHP_SockBuf result; result.reset(new SockBuf(audio_packet->Data(), audio_packet->Size()));
		baseCaller->SendTo(result);

		fileProgress = (fileProgress + 1) % mFile->Size();
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Point::Report(string type_)
{
	string result = "IPL\n";
	result += "From=ann\n";
	result += "To=" + Param("From") + "\n";
	result += type_ + "\n";
	result += "CallID=" + Param("CallID") + "\n";

	COM::SendModul(mapData["From"], result);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string Point::Param(string name_)
{
	if (name_ == "From") return mapData["From"];
	if (name_ == "Loop") return mapData["Loop"];
	if (name_ == "FileName") return mapData["FileName"];
	else return baseCaller->Param(name_);
}
string Point::PrintAll()
{
	//2TODO
	return "";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------