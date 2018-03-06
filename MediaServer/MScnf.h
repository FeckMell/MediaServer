#pragma once
#include "stdafx.h"
#include "SharedLib.h"
#include "MScnf_Cnf.h"


namespace cnf
{
	class Control
	{
	public:
		Control();
		void Preprocessing(string);


	private:
		void CR(SHP_IPL);
		void DL(SHP_IPL);

		SHP_Cnf FindCnf(string);
		void RemoveCnf(SHP_Cnf);

		vector<SHP_Cnf> vecCnf;
	};
	typedef shared_ptr<Control> SHP_Control;
}