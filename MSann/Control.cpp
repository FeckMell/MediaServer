#include "stdafx.h"
#include "Control.h"
using namespace ann;


Control::Control()
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Control::Control(), call to musicStore = make_shared<MusicStore>();";
	musicStore = make_shared<MusicStore>();
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Control::Control(): musicStore = make_shared<MusicStore>(); DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::CR(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Control::CR(...)";
	if (FindAnn(ipl_) != nullptr)
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::ann), fatal) << "Control::CR(...): FindAnn(ipl_) != nullptr for id=" << ipl_->data["EventID"];
		return;
	}
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Control::CR(...): SHP_MediaFile media_file = musicStore->GetFile(ipl_->data[IPL::fileName]);";
	SHP_MediaFile media_file = musicStore->GetFile(ipl_->data["FileName"]);
	if (media_file == nullptr) 
	{ 
		BOOST_LOG_SEV(LOG::GL(LOG::L::ann), fatal) << "Control::CR(...): media_file == nullptr for id=" << ipl_->data["EventID"];
		return;
	}
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Control::CR(...): SHP_Ann new_ann = make_shared<Ann>(media_file, ipl_);";
	SHP_Ann new_ann = make_shared<Ann>(media_file, ipl_);
	vecAnn.push_back(new_ann);
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Control::CR(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DL(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Control::DL(...)";
	SHP_Ann found_ann = FindAnn(ipl_);
	if (found_ann == nullptr)
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::ann), fatal) << "Control::CR(...): found_ann == nullptr for id=" << ipl_->data["EventID"];
		return;
	}
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Control::DL(...): found_ann->DL();";
	found_ann->DL();
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Control::DL(...): RemoveAnn(found_ann);";
	RemoveAnn(found_ann);
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "Control::DL(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Preprocessing(SHP_IPL ipl_)
{
	if (ipl_->data["EventType"] == "cr") CR(ipl_);
	else if (ipl_->data["EventType"] == "dl") DL(ipl_);
	else
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::ann), fatal) << "Control::Preprocessing(...): DEFAULT ERROR";
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