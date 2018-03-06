#pragma once
#include "stdafx.h"
#include "SharedLib.h"
#include "MSmgcp_MGCPparser.h"
#include "MSmgcp_Point.h"
#include "MSmgcp_Ann.h"
#include "MSmgcp_Cnf.h"


namespace mgcp
{
	class Control
	{
	public:

		Control();
		void PreprocessingIN(string);
		void PreprocessingOUT(REQUEST);

	private:

		void CRCX_CNF(SHP_MGCP);
		void CRCX_ANN(SHP_MGCP);

		void MDCX_CNF(SHP_MGCP);
		void RQNT_ANN(SHP_MGCP);

		void DLCX_CNF(SHP_MGCP);
		void DLCX_ANN(SHP_MGCP);

		SHP_Point FindPoint(string);
		void RemovePoint(SHP_Point);
		SHP_Cnf FindCnf(string);
		void RemoveCnf(SHP_Cnf);// delete ID
		SHP_Ann FindAnn(string);
		void RemoveAnn(SHP_Ann);// delete ID

		vector<SHP_Point> vecPoints;
		vector<SHP_Ann> vecAnns;
		vector<SHP_Cnf> vecCnfs;
	};
	typedef shared_ptr<Control> SHP_Control;
}