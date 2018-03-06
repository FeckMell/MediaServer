#include "stdafx.h"
#include "Control.h"
using namespace ann;


Control::Control()
{
	
	musicStore = make_shared<MusicStore>();
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::CR(SHP_IPL ipl_)
{
	
	if (FindAnn(ipl_) != nullptr)
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
	
	SHP_Ann found_ann = FindAnn(ipl_);
	if (found_ann == nullptr)
	{
		
		return;
	}
	
	found_ann->DL();
	
	RemoveAnn(found_ann);
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Preprocessing(SHP_IPL ipl_)
{
	if (ipl_->data["EventType"] == "cr") CR(ipl_);
	else if (ipl_->data["EventType"] == "dl") DL(ipl_);
	else
	{
		
		return;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Ann Control::FindAnn(SHP_IPL ipl_)
{
	for (auto& ann : vecAnn) if (ann->annID == ipl_->data["EventID"]) return ann;
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