#include "Control.h"

Control::Control()
{
	BOOST_LOG_SEV(lg, trace) << "Control::Control() and DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::CR(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(lg, trace) << "Control::CR(...)";
	if (FindCnf(ipl_) != nullptr)
	{
		BOOST_LOG_SEV(lg, fatal) << "Control::CR(...): ERROR";
		return;
	}
	BOOST_LOG_SEV(lg, trace) << "Control::CR(...): SHP_Cnf new_cnf = make_shared<Cnf>(ipl_);";
	SHP_Cnf new_cnf = make_shared<Cnf>(ipl_);
	vecCnf.push_back(new_cnf);
	BOOST_LOG_SEV(lg, trace) << "Control::CR(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::MD(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(lg, trace) << "Control::MD(...)";
	SHP_Cnf found_cnf = FindCnf(ipl_);
	if (found_cnf == nullptr)
	{
		BOOST_LOG_SEV(lg, fatal) << "Control::MD(...): ERROR";
		return;
	}
	BOOST_LOG_SEV(lg, trace) << "Control::MD(...): found_cnf->MD(ipl_);";
	found_cnf->MD(ipl_);
	BOOST_LOG_SEV(lg, trace) << "Control::MD(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DL(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(lg, trace) << "Control::DL(...)";
	SHP_Cnf found_cnf = FindCnf(ipl_);
	if (found_cnf == nullptr)
	{
		BOOST_LOG_SEV(lg, trace) << "Control::DL(..): ERROR";
		return;
	}
	BOOST_LOG_SEV(lg, trace) << "Control::DL(..): found_cnf->DL(ipl_);";
	found_cnf->DL(ipl_);
	BOOST_LOG_SEV(lg, trace) << "Control::DL(..): RemoveCnf(found_cnf);";
	RemoveCnf(found_cnf);
	BOOST_LOG_SEV(lg, trace) << "Control::DL(..) DONE";
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
	case IPL::md:
		MD(ipl_);
		break;
	case IPL::dl:
		DL(ipl_);
		break;
	default:
		BOOST_LOG_SEV(lg, fatal) << "Control::Preprocessing(..) DEFAULT";
		return;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Cnf Control::FindCnf(SHP_IPL ipl_)
{
	if (vecCnf.size() == 0) return nullptr;
	for (auto& cnf : vecCnf) if (cnf->cnfID == ipl_->data[IPL::eventID]) return cnf;
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