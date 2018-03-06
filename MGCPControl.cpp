#include "stdafx.h"
#include "MGCPcontrol.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
MGCPcontrol::MGCPcontrol()
{
	annControl.reset(new EventAnn());
	cnfControl.reset(new EventCnf());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCPcontrol::Preprocessing(SHP_MGCP mgcp_)
{
	switch (mgcp_->events[MGCP::CMD])
	{
	case MGCP::CRCX:
		switch (mgcp_->events[MGCP::Type])
		{
		case MGCP::ann: annControl->CRCX(mgcp_); break;
		case MGCP::cnf: cnfControl->CRCX(mgcp_); break;
		//case MGCP::prx: ; break;
		default: ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Option is unsupported"));
		}break;

	case MGCP::MDCX:
		switch (mgcp_->events[MGCP::Type])
		{
		case MGCP::cnf: cnfControl->MDCX(mgcp_);  break;
		//case MGCP::prx: ; break;
		default: ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Option is unsupported"));
		}break;

	case MGCP::DLCX:
		switch (mgcp_->events[MGCP::Type])
		{
		case MGCP::ann: annControl->DLCX(mgcp_); break;
		case MGCP::cnf: cnfControl->DLCX(mgcp_);  break;
		//case MGCP::prx: ; break;
		default: ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Option is unsupported"));
		}break;

	case MGCP::RQNT:
		switch (mgcp_->events[MGCP::Type])
		{
		case MGCP::ann: annControl->RQNT(mgcp_); break;
		default: ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Option is unsupported"));
		}break;

	default: ReplyClient(mgcp_, mgcp_->ResponseBAD(400, "Option is unsupported"));
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
