#include "stdafx.h"
#include "MScnf_Cnf.h"
using namespace cnf;


Cnf::Cnf(SHP_IPL ipl_)
{
	eventID = ipl_->data["EventID"];
	ParsePoints(ipl_);
	mixerAudio.reset(new Audio(vecCallers));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Cnf::~Cnf()
{
	mixerAudio.reset();
	vecCallers.clear();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::ParsePoints(SHP_IPL ipl_)
{
	unsigned i = 0;
	string call_id_num = "CallID" + to_string(i);
	//cout << "\nipl:" << ipl_->PrintAll() << "\n\n";
	while (ipl_->data[call_id_num] != "")
	{
		//cout << "\n" << call_id_num << "=" << ipl_->data[call_id_num] << "_";
		vecCallers.push_back(make_shared<Caller>(ipl_->data[call_id_num]));
		i++;
		call_id_num = "CallID" + to_string(i);
	}

}