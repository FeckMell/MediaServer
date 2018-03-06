#include "stdafx.h"
#include "MSdtmf.h"
using namespace dtmf;


MSdtmf::MSdtmf()
{
	innerServer.reset(new InnerServer());
	innerServer->Run();
	std::thread th([] { NET::RunIO(NET::INNER::dtmf); });
	th.detach();
}