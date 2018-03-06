#include "stdafx.h"
#include "MSann.h"

ann::MSann::MSann()
{
	innerServer.reset(new InnerServer());
	innerServer->Run();
	std::thread th([] { NET::RunIO(NET::INNER::ann); });
	th.detach();
}