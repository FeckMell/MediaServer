#pragma once
#include "SL_ALL.h"
#include "MSmgcp_Ann.h"
#include "MSmgcp_Cnf.h"

namespace mgcp
{
	class Control
	{
	public:

		Control();
		void PreprocessingINNER(string);
		void PreprocessingOUTTER();

	private:
		//OUT
		void CRCX_CNF(SHP_MGCP);
		void CRCX_ANN(SHP_MGCP);

		void MDCX_CNF(SHP_MGCP);
		void RQNT_ANN(SHP_MGCP);

		void DLCX_CNF(SHP_MGCP);
		void DLCX_ANN(SHP_MGCP);

		//IN
		void IN_ANN(SHP_IPL);

		//Common
		SHP_Cnf FindCnf(string);
		void RemoveCnf(SHP_Cnf);
		SHP_Ann FindAnn(string);
		void RemoveAnn(SHP_Ann);

		//Data
		vector<SHP_Ann> vecAnns;
		vector<SHP_Cnf> vecCnfs;
	};
	typedef shared_ptr<Control> SHP_Control;
}