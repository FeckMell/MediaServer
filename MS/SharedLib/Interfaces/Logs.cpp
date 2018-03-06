#include "stdafx.h"
#include "Logs.h"
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
map<string, ofstream> LOG::mapFile;
map<string, string> LOG::mapData;
map<string, int> LOG::mapLevel;
SHP_IO LOG::ioLog;
SHP_Socket LOG::fakeSocket;
SHP_thread LOG::actionThread;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LOG::Init()
{
	ioLog.reset(new IO());
	fakeSocket.reset(new Socket(CFG::Param("MyFakeIP"), CFG::Param("MyFakePort"), ioLog));
	fakeSocket->SetEndPoint(CFG::Param("OuterFakeIP"), CFG::Param("OuterFakePort"));
	fakeSocket->AsyncReceiveFrom(boost::bind(&LOG::FakeReceive, _1, _2));
	actionThread.reset(new thread([&](){ioLog->run(); cout << "\nThreadExit"; }));

	CheckDate();
	mapLevel["trace"] = 0;
	mapLevel["debug"] = 1;
	mapLevel["info"] = 2;
	mapLevel["info2"] = 3;
	mapLevel["error"] = 4;
	mapLevel["fatal"] = 5;
	mapLevel["Set"] = mapLevel[CFG::Param("LogLevel")];

	AddLogInstance("Errors");
	AddLogInstance("Shared");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LOG::AddLogInstance(string file_)
{
	mapFile[file_] = ofstream(CFG::Param("LogPath") + CFG::Param("Slash") + Param("Date") + "_" + file_ + ".log", ofstream::app);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string LOG::Param(string name_)
{
	return mapData[name_];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LOG::Log(string level_, string from_, string what_)
{
	string time_stamp = "\n\n\n" + TimeMS() + " :       ";
	string prefix = "\n\n\n" + TimeMS() + " : ThreadID=" + to_string(this_thread::get_id().hash()) + " :       ";
	ioLog->post(boost::bind(&LOG::Write, level_, from_, prefix + what_));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LOG::Write(string level_, string from_, string what_)
{
	if (mapLevel["Set"] > mapLevel[level_]) return;
	CheckDate();
	mapFile[from_].write(what_.c_str(), what_.length());
	mapFile[from_].flush();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LOG::ReinitFiles()
{
	mapData["Date"] = to_iso_extended_string(boost::gregorian::day_clock::universal_day());
	for (auto& e : mapFile)
	{
		e.second.close();
		e.second.open(CFG::Param("LogPath") + CFG::Param("Slash") + Param("Date") + "_" + e.first + ".log", ofstream::app);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string LOG::TimeMS()
{
	return to_iso_extended_string(boost::posix_time::microsec_clock::local_time());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LOG::CheckDate()
{
	if (Param("Date") != to_iso_extended_string(boost::gregorian::day_clock::universal_day()))
		ReinitFiles();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LOG::FakeReceive(boost::system::error_code ec_, size_t size_)
{
	if (!ec_)
	{
		this_thread::sleep_for(chrono::milliseconds(5)); //DebugTest
		fakeSocket->AsyncReceiveFrom(boost::bind(&LOG::FakeReceive, _1, _2));
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------