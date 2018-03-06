#include "stdafx.h"
#include "MSann.h"
using namespace ann;


Control::Control()
{
	musicStore = make_shared<MusicStore>();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::CR(SHP_IPL ipl_)
{
	if (FindAnn(ipl_->data["EventID"]) != nullptr)
	{
		return;
	}
	
	SHP_MediaFile media_file = musicStore->GetFile(ipl_->data["FileName"]);
	if (media_file == nullptr) 
	{ 
		return;
	}
	
	SHP_Ann new_ann = make_shared<Ann>(media_file, ipl_);
	vecAnn.push_back(new_ann);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DL(SHP_IPL ipl_)
{
	SHP_Ann found_ann = FindAnn(ipl_->data["EventID"]);
	if (found_ann == nullptr)
	{
		return;
	}
	
	found_ann->DL();
	RemoveAnn(found_ann);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Preprocessing(string message_)
{
	SHP_IPL ipl = make_shared<IPL>(IPL(message_));

	if (ipl->data["EventType"] == "cr") CR(ipl);
	else if (ipl->data["EventType"] == "dl") DL(ipl);
	else
	{
		return;
	} 
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Ann Control::FindAnn(string event_id_)
{
	for (auto& ann : vecAnn) if (ann->eventID == event_id_) return ann;
	return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::RemoveAnn(SHP_Ann ann_)
{
	vecAnn.erase(remove(vecAnn.begin(), vecAnn.end(), ann_), vecAnn.end());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------