#pragma once
#include "../../stdafx.h"

class Timer
{
public:
	Timer();

	void Update();
	string PrintAll();

	boost::posix_time::ptime GetLast();
	boost::posix_time::ptime GetStart();
	boost::posix_time::time_duration GetFullDuration();
	boost::posix_time::time_duration GetActiveDuration();
	boost::posix_time::time_duration GetInactiveTime();

private:

	boost::posix_time::ptime start;
	boost::posix_time::ptime last;

};
typedef shared_ptr<Timer> SHP_Timer;