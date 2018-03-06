#pragma once
#include "SL_ALL.h"

namespace sip
{
	class Caller
	{
	public:

		enum State{ login, pass, ready, paused };
		Caller(string, SHP_SDP, SHP_SDP);

		string GetParam(string);
		void DTMFResult(SHP_IPL);

		void SendToAnnModul(string, string);
		void SendToDTMFModul(string);
		void StopAll();

		bool playingAnn = false;
		bool listenDTMF = false;
		int state = login;

	private:

		string roomID;
		string eventID;
		SHP_Point basePoint;

	};
	typedef shared_ptr<Caller> SHP_Caller;
}