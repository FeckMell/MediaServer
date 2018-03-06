#include "stdafx.h"
#include "SharedLib.h"
#include "MSreceiver.h"

#include "MSann.h"
#include "MScnf.h"
#include "MSsip.h"
#include "MSmgcp.h"
#include "MSdtmf.h"

int main(int argc, char* argv[])
{
	//timeBeginPeriod(1);//CPU timer 1ms
	//SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);//Realtime priority in system
	setlocale(LC_ALL, "Russian");//Russia location
	CFG::Init(argv[0]);
	LOG::Init();
	NET::Init();
	LOG::Log(LOG::fatal, "MAIN", "MAIN Init done with params:\n" + CFG::GetParams());
	cout << "\nINIT DONE";
	
	ann::Control ann;
	cnf::Control cnf;
	mgcp::Control mgcp;

	dtmf::Control dtmf;
	sip::Control sip;
	
	NET::vecSigsIN[NET::INNER::ann].connect(boost::bind(&ann::Control::Preprocessing, &ann, _1));
	NET::vecSigsIN[NET::INNER::cnf].connect(boost::bind(&cnf::Control::Preprocessing, &cnf, _1));
	NET::vecSigsIN[NET::INNER::dtmf].connect(boost::bind(&dtmf::Control::Preprocessing, &dtmf, _1));

	NET::vecSigsIN[NET::INNER::sip].connect(boost::bind(&sip::Control::PreprocessingIN, &sip, _1));
	NET::vecSigsIN[NET::INNER::mgcp].connect(boost::bind(&mgcp::Control::PreprocessingIN, &mgcp, _1));

	NET::vecSigsOUT[NET::OUTER::sip_].connect(boost::bind(&sip::Control::PreprocessingOUT, &sip, _1));
	NET::vecSigsOUT[NET::OUTER::mgcp_].connect(boost::bind(&mgcp::Control::PreprocessingOUT, &mgcp, _1));

	Receiver receiver;
	
	cout << "\n1";
	system("pause");
	return 0;
}