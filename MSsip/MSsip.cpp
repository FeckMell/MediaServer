#include "stdafx.h"
#include "MSsip.h"
using namespace sip;


MSsip::MSsip()
{
	sipServer.reset(new SIPServer());
	sipServer->Run();
	std::thread th([] { NET::RunIO(NET::INNER::sip_i); });
	th.detach();
	std::thread th2([] { NET::RunIO(NET::OUTER::sip); });
	th2.detach();
}