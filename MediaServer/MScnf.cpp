#include "stdafx.h"
#include "MScnf.h"
using namespace cnf;


Control::Control()
{
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Preprocessing(string message_)
{
	SHP_IPL ipl = make_shared<IPL>(IPL(message_));
	BOOST_LOG_SEV(LOG::GL(0), info) << "MSCNF ipl=\n" << message_;
	if (ipl->data["EventType"] == "cr") CR(ipl);
	else if (ipl->data["EventType"] == "dl") DL(ipl);
	else
	{
		BOOST_LOG_SEV(LOG::GL(0), fatal) << "MSCNF: preprocess ERROR";
		return;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::CR(SHP_IPL ipl_)
{
	SHP_Cnf found_cnf = FindCnf(ipl_->data["EventID"]);
	if (found_cnf != nullptr)
	{ 
		found_cnf->~Cnf();
		RemoveCnf(found_cnf); 
	}

	SHP_Cnf new_cnf = make_shared<Cnf>(ipl_);
	vecCnf.push_back(new_cnf);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DL(SHP_IPL ipl_)
{
	SHP_Cnf found_cnf = FindCnf(ipl_->data["EventID"]);
	if (found_cnf == nullptr)
	{
		BOOST_LOG_SEV(LOG::GL(0), fatal) << "MSCNF: DL ERROR";
		return;
	}
	else
	{
		found_cnf->~Cnf();
		RemoveCnf(found_cnf);
	}	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Cnf Control::FindCnf(string event_id_)
{
	for (auto& cnf : vecCnf) if (cnf->eventID == event_id_) return cnf;
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