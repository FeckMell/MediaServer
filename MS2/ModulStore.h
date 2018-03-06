#pragma once
#include "SL_All.h"

class ModulStore
{
public:

	//static void Init(); /* init all: cfg, com, log */
	static void Start();

	static void GetLogic(string modulname_, string logicname_);
	static void RegisterModul(string modulname_, map<string, boost::any> data_);

private:

	static map<string, map<string, boost::any>> _data;
	/*map<string MODUL_NAME, map<string STORED_PARAM_NAME, boost::any PARAM>> _data;*/ 
	/*
	boost::any PARAM :
		1) Modul object
		2) main func
		3) support func1
		4) support func2
		...
	*/

};