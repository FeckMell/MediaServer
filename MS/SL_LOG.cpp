#include "stdafx.h"
#include "SL_LOG.h"

map<string, ofstream> LOG::files;
SHP_SOCK LOG::socket;
vector<string> LOG::fileNames = { "MAIN", "ANN", "CNF", "DTMF", "MGCP", "SIP", "ERRORS" };
string LOG::lastDate = "";
SHP_thread LOG::th;

void LOG::Init()
{
	string time_prefix = to_iso_extended_string(boost::gregorian::day_clock::universal_day());
	string path = CFG::data["logPath"];
	for (auto& e : fileNames)
		files[e].open(path + CFG::slash + time_prefix + "_" + e + ".log", ofstream::app);

	
	/*Set fake recieve to make boost::asio::io_service wait for events*/
	SHP_IO new_io; new_io.reset(new IO());
	socket.reset(new SOCK("127.0.0.1", stoi(CFG::data["logPort"]), new_io));
	socket->SetEndPoint("10.10.10.10", "45821");
	socket->AsyncReceive(boost::bind(&LOG::FakeReceive, _1, _2));

	th.reset(new thread([]{ socket->io->run(); }));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LOG::Log(int lvl_, string where_, string what_)
{
	socket->io->post(boost::bind(LOG::Write, lvl_, where_, what_));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LOG::Write(int lvl_, string where_, string what_)
{
	if (lvl_ <= stoi(CFG::data["logLevel"])) return;
	CheckDate();
	string time_stamp = "\n\n\n" + TimeMS() + " :       ";
	what_ = time_stamp + what_;

	files[where_].write(what_.c_str(), what_.length());
	files[where_].flush();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LOG::ReinitFiles()
{
	string time_prefix = to_iso_extended_string(boost::gregorian::day_clock::universal_day());
	string path = CFG::data["logPath"];
	for (auto& e : fileNames)
	{
		files[e].close();
		files[e].open(path + CFG::slash + time_prefix + "_" + e + ".log", ofstream::app);
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
	if (lastDate != to_iso_extended_string(boost::gregorian::day_clock::universal_day()))
		ReinitFiles();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LOG::FakeReceive(boost::system::error_code ec_, size_t size_)
{
	socket->AsyncReceive(boost::bind(&LOG::FakeReceive, _1, _2));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------

