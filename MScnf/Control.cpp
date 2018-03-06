#include "stdafx.h"
#include "Control.h"
using namespace cnf;


Control::Control()
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Control::Control() and DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::CR(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Control::CR(...)";
	if (FindCnf(ipl_) != nullptr)
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), fatal) << "Control::CR(...): ERROR";
		return;
	}
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Control::CR(...): SHP_Cnf new_cnf = make_shared<Cnf>(ipl_);";
	SHP_Cnf new_cnf = make_shared<Cnf>(ipl_);
	vecCnf.push_back(new_cnf);
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Control::CR(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::MD(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Control::MD(...)";
	SHP_Cnf found_cnf = FindCnf(ipl_);
	if (found_cnf == nullptr)
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), fatal) << "Control::MD(...): ERROR";
		return;
	}
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Control::MD(...): found_cnf->MD(ipl_);";
	found_cnf->MD(ipl_);
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Control::MD(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DL(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Control::DL(...)";
	SHP_Cnf found_cnf = FindCnf(ipl_);
	if (found_cnf == nullptr)
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Control::DL(..): ERROR";
		return;
	}
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Control::DL(..): found_cnf->DL(ipl_);";
	found_cnf->DL(ipl_);
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Control::DL(..): RemoveCnf(found_cnf);";
	RemoveCnf(found_cnf);
	BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), trace) << "Control::DL(..) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Preprocessing(SHP_IPL ipl_)
{
	if (ipl_->data["EventType"] == "cr") CR(ipl_);
	else if (ipl_->data["EventType"] == "md") MD(ipl_);
	else if (ipl_->data["EventType"] == "dl") DL(ipl_);
	else 
	{
		BOOST_LOG_SEV(LOG::GL(LOG::L::cnf), fatal) << "Control::Preprocessing(..) DEFAULT";
		return;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Cnf Control::FindCnf(SHP_IPL ipl_)
{
	for (auto& cnf : vecCnf) if (cnf->cnfID == ipl_->data["EventID"]) return cnf;
	return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::RemoveCnf(SHP_Cnf cnf_)
{
	vecCnf.erase(remove(vecCnf.begin(), vecCnf.end(), cnf_), vecCnf.end());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------