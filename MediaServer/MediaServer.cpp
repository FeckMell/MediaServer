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
	timeBeginPeriod(1);//CPU timer 1ms
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);//Realtime priority in system
	setlocale(LC_ALL, "Russian");//Russia location

	CFG::Init(argv[0]);
	LOG::Init();
	NET::Init();
	cout << "\n INIT DONE\n";

	
	ann::Control ann;
	cnf::Control cnf;
	dtmf::Control dtmf;
	sip::Control sip;
	mgcp::Control mgcp;
	NET::vecSigsIN[NET::INNER::ann].connect(boost::bind(&ann::Control::Preprocessing, &ann, _1));
	NET::vecSigsIN[NET::INNER::cnf].connect(boost::bind(&cnf::Control::Preprocessing, &cnf, _1));
	NET::vecSigsIN[NET::INNER::dtmf].connect(boost::bind(&dtmf::Control::Preprocessing, &dtmf, _1));

	NET::vecSigsIN[NET::INNER::sip].connect(boost::bind(&sip::Control::PreprocessingIN, &sip, _1));
	NET::vecSigsIN[NET::INNER::mgcp].connect(boost::bind(&mgcp::Control::PreprocessingIN, &mgcp, _1));

	NET::vecSigsOUT[NET::OUTER::sip_].connect(boost::bind(&sip::Control::PreprocessingOUT, &sip, _1));
	NET::vecSigsOUT[NET::OUTER::mgcp_].connect(boost::bind(&mgcp::Control::PreprocessingOUT, &mgcp, _1));

	Receiver receiver;
		
	//ann::SHP_MSann ann = make_shared<ann::MSann>();
	//cnf::SHP_MScnf cnf = make_shared<cnf::MScnf>();
	//dtmf::SHP_MSdtmf dtmf = make_shared<dtmf::MSdtmf>();
	//mgcp::SHP_MSmgcp mgcp = make_shared<mgcp::MSmgcp>();
	//sip::SHP_MSsip sip = make_shared<sip::MSsip>();
	
	cout << "\n1";
	system("pause");
	return 0;
}