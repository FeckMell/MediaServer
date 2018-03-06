#include "stdafx.h"
#include "SL_Logs.h"

map<string, ofstream> LOG::files;
SHP_SOCK LOG::socket;
vector<string> LOG::fileNames = { "MAIN", "ANN", "CNF", "DTMF", "MGCP", "SIP" };
string LOG::lastDate = "";
uint8_t LOG::fake_data[10];

void LOG::Init()
{
	string time_prefix = to_iso_extended_string(boost::gregorian::day_clock::universal_day());
	string path = CFG::data["logPath"];
	for (auto& e : fileNames)
		files[e].open(path + CFG::slash + time_prefix + "_" + e + ".log", ofstream::app);

	EP endPoint = EP(boost::asio::ip::address::from_string("10.10.10.10"), stoi("45821"));
	SHP_IO new_io; new_io.reset(new IO());
	socket.reset(new SOCK("127.0.0.1", stoi(CFG::data["logPort"]), new_io));
	socket->s.async_receive_from(boost::asio::buffer(fake_data, 10), endPoint, boost::bind(&LOG::FakeReceive, _1, _2));

	thread th([]{cout << "\nLOG:IO"; socket->io->run(); cout << "\nLOG::IO F"; });
	th.detach();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LOG::Log(int lvl_,string where_, string what_)
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
	EP endPoint = EP(boost::asio::ip::address::from_string("10.10.10.10"), stoi("45821"));
	socket->s.async_receive_from(boost::asio::buffer(fake_data, 10),
		endPoint,
		boost::bind(&LOG::FakeReceive, _1, _2)
		);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------

