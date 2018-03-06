#pragma once
#include "stdafx.h"
#include "Cnf.h"

extern SHP_STARTUP init_Params;

class Control
{
public:
	Control();
	void Preprocessing(SHP_IPL);


private:
	void CR(SHP_IPL);
	void MD(SHP_IPL);
	void DL(SHP_IPL);

	SHP_Cnf FindCnf(SHP_IPL);
	void RemoveCnf(SHP_Cnf);

	vector<SHP_Cnf> vecCnf;
};
typedef shared_ptr<Control> SHP_Control;