#include "stdafx.h"
#include "MSann.h"
using namespace ann;


Control::Control()
{
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
		LOG::Log(LOG::fatal, "ERRORS", "MSANN: Preprocess ERROR 1");
		return;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::CR(SHP_IPL ipl_)
{
	if (FindAnn(ipl_->data["EventID"]) != nullptr)
	{
		LOG::Log(LOG::fatal, "ERRORS", "MSANN: CR ERROR 1");
		return;
	}
	SHP_MediaFile media_file = MusicStore::GetFile(ipl_->data["FileName"]);
	if (media_file == nullptr)
	{
		LOG::Log(LOG::fatal, "ERRORS", "MSANN: CR ERROR 2");
		return;
	}

	SHP_Ann new_ann; new_ann.reset(new Ann(media_file, ipl_));
	vecAnn.push_back(new_ann);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DL(SHP_IPL ipl_)
{
	SHP_Ann found_ann = FindAnn(ipl_->data["EventID"]);
	if (found_ann == nullptr)
	{
		LOG::Log(LOG::fatal, "ERRORS", "MSANN: DL ERROR 1");
		return;
	}

	found_ann->DL();
	RemoveAnn(found_ann);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Ann Control::FindAnn(string event_id_)
{
	for (auto& ann : vecAnn) if (ann->GetParam("EventID") == event_id_) return ann;
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