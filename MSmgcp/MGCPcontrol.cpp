#include "stdafx.h"
#include "MGCPcontrol.h"

MGCPcontrol::MGCPcontrol()
{
	BOOST_LOG_SEV(lg, trace) << "MGCPcontrol::MGCPcontrol()";
	annControl.reset(new EventAnn());
	BOOST_LOG_SEV(lg, trace) << "MGCPcontrol::MGCPcontrol(): annControl.reset(new EventAnn()); DONE";
	cnfControl.reset(new EventCnf());
	BOOST_LOG_SEV(lg, trace) << "MGCPcontrol::MGCPcontrol(): cnfControl.reset(new EventCnf()); DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MGCPcontrol::Preprocessing(SHP_MGCP mgcp_)
{
	BOOST_LOG_SEV(lg, trace) << "MGCPcontrol::Preprocessing(...)";
	switch (mgcp_->events[MGCP::CMD])
	{
	case MGCP::CRCX:
		switch (mgcp_->events[MGCP::Type])
		{
		case MGCP::ann: annControl->CRCX(mgcp_); break;
		case MGCP::cnf: cnfControl->CRCX(mgcp_); break;
		//case MGCP::prx: ; break;
		default: mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, "Option is unsupported"));
		}break;

	case MGCP::MDCX:
		switch (mgcp_->events[MGCP::Type])
		{
		case MGCP::cnf: cnfControl->MDCX(mgcp_);  break;
		//case MGCP::prx: ; break;
		default: mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, "Option is unsupported"));
		}break;

	case MGCP::DLCX:
		switch (mgcp_->events[MGCP::Type])
		{
		case MGCP::ann: annControl->DLCX(mgcp_); break;
		case MGCP::cnf: cnfControl->DLCX(mgcp_);  break;
		//case MGCP::prx: ; break;
		default: mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, "Option is unsupported"));
		}break;

	case MGCP::RQNT:
		switch (mgcp_->events[MGCP::Type])
		{
		case MGCP::ann: annControl->RQNT(mgcp_); break;
		default: mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, "Option is unsupported"));
		}break;

	default: mgcp_->ReplyClient(net_Data->GS(NETDATA::out), mgcp_->ResponseBAD(400, "Option is unsupported"));
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
