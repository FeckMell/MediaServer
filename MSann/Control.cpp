#include "Control.h"

Control::Control()
{
	BOOST_LOG_SEV(lg, trace) << "Control::Control(), call to musicStore = make_shared<MusicStore>();";
	musicStore = make_shared<MusicStore>();
	BOOST_LOG_SEV(lg, trace) << "Control::Control(): musicStore = make_shared<MusicStore>(); DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::CR(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(lg, trace) << "Control::CR(...)";
	if (FindAnn(ipl_) != nullptr)
	{
		BOOST_LOG_SEV(lg, fatal) << "Control::CR(...): FindAnn(ipl_) != nullptr for id=" << ipl_->data[IPL::eventID];
		return;
	}
	BOOST_LOG_SEV(lg, trace) << "Control::CR(...): SHP_MediaFile media_file = musicStore->GetFile(ipl_->data[IPL::fileName]);";
	SHP_MediaFile media_file = musicStore->GetFile(ipl_->data[IPL::fileName]);
	if (media_file == nullptr) 
	{ 
		BOOST_LOG_SEV(lg, fatal) << "Control::CR(...): media_file == nullptr for id=" << ipl_->data[IPL::eventID];
		return;
	}
	BOOST_LOG_SEV(lg, trace) << "Control::CR(...): SHP_Ann new_ann = make_shared<Ann>(media_file, ipl_);";
	SHP_Ann new_ann = make_shared<Ann>(media_file, ipl_);
	vecAnn.push_back(new_ann);
	BOOST_LOG_SEV(lg, trace) << "Control::CR(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DL(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(lg, trace) << "Control::DL(...)";
	SHP_Ann found_ann = FindAnn(ipl_);
	if (found_ann == nullptr)
	{
		BOOST_LOG_SEV(lg, fatal) << "Control::CR(...): found_ann == nullptr for id=" << ipl_->data[IPL::eventID];
		return;
	}
	BOOST_LOG_SEV(lg, trace) << "Control::DL(...): found_ann->DL();";
	found_ann->DL();
	BOOST_LOG_SEV(lg, trace) << "Control::DL(...): RemoveAnn(found_ann);";
	RemoveAnn(found_ann);
	BOOST_LOG_SEV(lg, trace) << "Control::DL(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Preprocessing(SHP_IPL ipl_)
{
	switch (ipl_->type)
	{
	case IPL::cr:
		CR(ipl_);
		break;
	case IPL::dl:
		DL(ipl_);
		break;
	default:
		BOOST_LOG_SEV(lg, fatal) << "Control::Preprocessing(...): DEFAULT ERROR";
		return;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Ann Control::FindAnn(SHP_IPL ipl_)
{
	for (auto& ann : vecAnn) if (ann->annID == ipl_->data[IPL::eventID]) return ann;
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