#include "stdafx.h"
#include "MScnf.h"

cnf::MScnf::MScnf()
{
	innerServer.reset(new InnerServer());
	innerServer->Run();
	std::thread th([] { NET::RunIO(NET::INNER::cnf); });
	th.detach();
}