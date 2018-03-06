#pragma once
#include "stdafx.h"
#include "SharedLib.h"
#include "MSsip_SIPparser.h"


namespace sip
{
	class Point
	{
	public:

		enum State{ login, pass, ready, paused };
		Point(SHP_SIP, string);

		void DTMFResult(SHP_IPL);

		void PlayAnn(string);
		void StopAnn();
		void ListenDTMF();
		void StopDTMF();
		void StopAll();

		SHP_SDP serverSDP;
		SHP_SDP clientSDP;

		string callID;

		string eventID;
		string roomID;
		int state = login;
		bool playingAnn = false;
	private:

	};
	typedef shared_ptr<Point> SHP_Point;
}