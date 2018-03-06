#include "stdafx.h"
#include "CallerAnn.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
CallerAnn::CallerAnn(SHP_MGCP mgcp_, string server_sdp_, string server_port_)
:CallerBase(mgcp_, server_sdp_, server_port_)
{
	eventNum = mgcp_->data[MGCP::EventNum];
	ReplyClient(mgcp_, mgcp_->ResponseOK(200,"add event type") + "\n\n" + server_sdp_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void CallerAnn::RQNT(SHP_MGCP mgcp_)
{
	filename = get_substr(mgcp_->data[MGCP::ParamS], ",file:///", ")");
	//cout << "\nDEBUG RQNT::filename=" << filename << "_";
	state = "play";
	ReplyClient(mgcp_, mgcp_->ResponseOK(200, ""));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void CallerAnn::DLCX(SHP_MGCP mgcp_)
{
	filename = "";
	state = "delete";
	ReplyClient(mgcp_, mgcp_->ResponseOK(250, ""));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------