#include "stdafx.h"
#include "SIPControl.h"

void SIPControl::loggit(string a)
{
	time_t rawtime;
	struct tm * t;
	time(&rawtime);
	t = localtime(&rawtime);
	string time = "";
	steady_clock::time_point t1 = steady_clock::now();
	time += DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
	time += " thread=" + boost::to_string(this_thread::get_id());
	CLogger.AddToLog(9, "\n" + time + "     " + a);
}