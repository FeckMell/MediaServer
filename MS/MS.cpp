// MS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SharedLib/All.h"
#include "Logic/MGCP/LMGCP.h"
#include "Functional/FAnn/FAnn.h"
#include "Functional/FCnf/FCnf.h"
//#include "Logs.h"


int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");//Russia location
	/*Basis*/
	CFG::Init(argv[0]);
	LOG::Init();
	CallerStore::Init();
	/*\Basis*/

	/*Logic*/
	NLmgcp::LMGCP::Init();
	/*\Logic*/

	/*Functional*/
	NFAnn::FAnn::Init();
	NFCnf::FCnf::Init();
	/*\Functional*/

	/*Utils*/
	/*\Utils*/

	this_thread::sleep_for(chrono::hours(2));
	//system("pause");
	return 0;
}

