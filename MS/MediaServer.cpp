#include "stdafx.h"
#include "SL_ALL.h"
#include "MSreceiver.h"
#include "MSmgcp.h"
#include "MSsip.h"
#include "MSann.h"
#include "MScnf.h"

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");//Russia location
	CFG::Init(argv[0]);
	LOG::Init();
	COM::Init();
	LOG::Log(LOG::fatal, "MAIN", "MAIN Init done with params:\n" + CFG::GetParams());
	cout << "\nINIT DONE";

	ann::Control ann;
	cnf::Control cnf;
	mgcp::Control mgcp;

	//dtmf::Control dtmf;
	sip::Control sip;

	COM::vecSigsIN[COM::INNER::ann].connect(boost::bind(&ann::Control::Preprocessing, &ann, _1));
	COM::vecSigsIN[COM::INNER::cnf].connect(boost::bind(&cnf::Control::Preprocessing, &cnf, _1));
	//COM::vecSigsIN[COM::INNER::dtmf].connect(boost::bind(&dtmf::Control::Preprocessing, &dtmf, _1));

	COM::vecSigsIN[COM::INNER::sip].connect(boost::bind(&sip::Control::PreprocessingINNER, &sip, _1));
	COM::vecSigsIN[COM::INNER::mgcp].connect(boost::bind(&mgcp::Control::PreprocessingINNER, &mgcp, _1));

	COM::vecSigsOUT[COM::OUTER::sip_].connect(boost::bind(&sip::Control::PreprocessingOUTTER, &sip));
	COM::vecSigsOUT[COM::OUTER::mgcp_].connect(boost::bind(&mgcp::Control::PreprocessingOUTTER, &mgcp));

	Receiver receiver;

	cout << "\n1";
	system("pause");
	return 0;
}