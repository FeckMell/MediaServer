#include "stdafx.h"
#include "Structs.h"

IPar::IPar(char** argv_, string modulnamestr_)
{
	modulName = modulnamestr_;
	data.resize(maxParamNames);
	for (int i = 1; i <= maxParamNames; ++i)
		data[i - 1] = argv_[i];
}
//*///------------------------------------------------------------------------------------------
string IPar::GetParams()
{
	string result = "";

	result += "\nNumber, from which RTP ports starts counting with step = 2: " + data[rtpPort];
	result += "\nOuter port: " + data[outerPort];
	result += "\nInner port: " + data[innerPort];
	result += "\nInner IP: " + data[innerIP];
	result += "\nMediaPath: " + data[mediaPath];
	result += "\nOuter IP: " + data[outerIP];
	result += "\nMax inactive time for Announcements in mins: " + data[maxTimeAnn];
	result += "\nMax inactive time for Conference in mins: " + data[maxTimeCnf];
	result += "\nMax inactive time for Proxy calls in mins: " + data[maxTimePrx];
	result += "\nLogLevel: " + data[logLevel];
	result += "\nParsed path to application(home folder for MGCP server):\n" + data[homePath];
	return result + "\n";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Socket::Socket(string ip_, int port_, IO& io_) :s(io_)
{
	using boost::asio::ip::udp;
	s.open(udp::v4());
	s.set_option(udp::socket::reuse_address(true));
	s.bind(EP(boost::asio::ip::address::from_string(ip_), port_));
	BOOST_LOG_SEV(lg, trace) << "Socket::Socket(...) created with IP=" << ip_ << " port=" << port_;
}
Socket::~Socket()
{
	BOOST_LOG_SEV(lg, trace) << "Socket::Socket(...) CLOSED with IP=" << s.local_endpoint().address().to_string() << " port=" << s.local_endpoint().port();
	s.close();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
RTP_struct::RTP_struct()
{
	Config();
}
//*///------------------------------------------------------------------------------------------
void RTP_struct::Config()
{
	this->header.version = 2;
	this->header.marker = 0;
	this->header.csrc_len = 0;
	this->header.extension = 0;
	this->header.padding = 0;
	this->header.ssrc = htons(10);
	this->header.payload_type = 8;
	this->header.timestamp = htonl(0);
	this->header.seq_no = htons(0);
}
//*///------------------------------------------------------------------------------------------
RTP RTP_struct::Get()
{
	++this->amount;
	this->header.seq_no = htons(this->amount);
	this->header.timestamp = htonl(160 * this->amount);

	return header;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
CAVPacket::CAVPacket()
{
	av_init_packet(&packet);
	packet.data = nullptr;
	packet.size = 0;
}
//*///------------------------------------------------------------------------------------------
CAVPacket::CAVPacket(size_t sz)
{
	if (sz > 0)
		av_new_packet(&packet, sz);
	else
	{
		av_init_packet(&packet);
		packet.data = nullptr;
		packet.size = 0;
	}
}
//*///------------------------------------------------------------------------------------------
CAVPacket::~CAVPacket()
{
	Free();
}
//*///------------------------------------------------------------------------------------------
AVPacket* CAVPacket::Get()
{
	return &packet;
}
//*///------------------------------------------------------------------------------------------
int CAVPacket::Size()
{
	return packet.size;
}
//*///------------------------------------------------------------------------------------------
uint8_t* CAVPacket::Data()
{
	return packet.data;
}
//*///------------------------------------------------------------------------------------------
void CAVPacket::Free()
{
	av_free_packet(&packet);
}
//*///------------------------------------------------------------------------------------------
void CAVPacket::MakeSize(int n)
{
	packet.size = n;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
CAVFrame::CAVFrame()
{
	frame = av_frame_alloc();
	empty = false;
}
//*///------------------------------------------------------------------------------------------
CAVFrame::CAVFrame(bool a)
{
	frame = av_frame_alloc();
	empty = true;
}
//*///------------------------------------------------------------------------------------------
CAVFrame::~CAVFrame()
{
	av_frame_free(&frame);
}
//*///------------------------------------------------------------------------------------------
AVFrame* CAVFrame::Get()
{
	return frame;
}
//*///------------------------------------------------------------------------------------------
void CAVFrame::Free()
{
	av_frame_free(&frame);
}
//*///------------------------------------------------------------------------------------------
bool CAVFrame::Empty()
{
	return empty;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
NETDATA::NETDATA(int mymodulnameint_)
{
	BOOST_LOG_SEV(lg, trace) << "NETDATA::NETDATA(): sockets.resize=" << maxS;
	sockets.resize(maxS);
	BOOST_LOG_SEV(lg, trace) << "NETDATA::NETDATA(): endPoints.resize=" << maxE;
	endPoints.resize(maxE);
	BOOST_LOG_SEV(lg, trace) << "NETDATA::NETDATA(): ios.resize=" << maxS;
	ios.resize(maxS);
	BOOST_LOG_SEV(lg, trace) << "NETDATA::NETDATA(): resize DONE";
	for (int i = 0; i < maxS; ++i) ios[i].reset(new IO());
	sockets[out].reset(new Socket(
		init_Params->data[IPar::outerIP],
		stoi(init_Params->data[IPar::outerPort]),
		GI(out)
		));
	BOOST_LOG_SEV(lg, debug) << "NETDATA::NETDATA(): opened socket with IP=" << init_Params->data[IPar::outerIP] << " Port=" << stoi(init_Params->data[IPar::outerPort]);
	sockets[in].reset(new Socket(
		init_Params->data[IPar::innerIP],
		stoi(init_Params->data[IPar::innerPort]) + mymodulnameint_ - 1,
		GI(in)
		));
	BOOST_LOG_SEV(lg, debug) << "NETDATA::NETDATA(): opened socket with IP=" << init_Params->data[IPar::innerIP] << " Port=" << stoi(init_Params->data[IPar::innerPort]) + mymodulnameint_ - 1;
	for (int i = 0; i < maxE; ++i)
	{
		endPoints[i] = EP(
			boost::asio::ip::address::from_string(init_Params->data[IPar::innerIP]),
			stoi(init_Params->data[IPar::innerPort]) + i - 1
			);
		BOOST_LOG_SEV(lg, debug) << "NETDATA::NETDATA():Created EndPoint for IP=" << init_Params->data[IPar::innerIP] << " Port=" << stoi(init_Params->data[IPar::innerPort]) + i - 1;
	}
}
SHP_Socket NETDATA::GS(int s_)
{
	try { return sockets[s_]; }
	catch (exception& e)
	{
		BOOST_LOG_SEV(lg, fatal) << "NETDATA::GetSocket: " << e.what();
		system("pause");
		exit(-1);
	}
}
EP NETDATA::GE(int e_)
{
	try { return endPoints[e_]; }
	catch (exception& e)
	{
		BOOST_LOG_SEV(lg, fatal) << "NETDATA::GetEndPoint: " << e.what();
		system("pause");
		exit(-1);
	}
}
IO& NETDATA::GI(int io_)
{
	try { return *ios[io_].get(); }
	catch (exception& e)
	{
		BOOST_LOG_SEV(lg, fatal) << "NETDATA::GetService: " << e.what();
		system("pause");
		exit(-1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------