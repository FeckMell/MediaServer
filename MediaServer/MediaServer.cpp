#include "stdafx.h"

int main(int argc, char* argv[])
{
	timeBeginPeriod(1);//CPU timer 1ms
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);//Realtime priority in system
	setlocale(LC_ALL, "Russian");//Russia location

	CFG::Init(argv[0]);
	LOG::Init();
	NET::Init();
	cout << "\n INIT DONE\n";

	ann::SHP_MSann ann = make_shared<ann::MSann>();
	cnf::SHP_MScnf cnf = make_shared<cnf::MScnf>();
	dtmf::SHP_MSdtmf dtmf = make_shared<dtmf::MSdtmf>();
	mgcp::SHP_MSmgcp mgcp = make_shared<mgcp::MSmgcp>();
	sip::SHP_MSsip sip = make_shared<sip::MSsip>();

	system("pause");
	return 0;
}