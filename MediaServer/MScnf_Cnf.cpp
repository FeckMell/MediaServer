#include "stdafx.h"
#include "MScnf_Cnf.h"
using namespace cnf;


Cnf::Cnf(SHP_IPL ipl_)
{
	eventID = ipl_->data["EventID"];
	ParsePoints(ipl_);
	mixerAudio.reset(new Audio(vecPoints));
}
Cnf::~Cnf()
{
	mixerAudio.reset();
	vecPoints.clear(); 
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::ParsePoints(SHP_IPL ipl_)
{
	vector<string> client_ports = ParseLine(ipl_->data["ClientPort"]);
	vector<string> server_ports = ParseLine(ipl_->data["ServerPort"]);
	vector<string> client_ips = ParseLine(ipl_->data["ClientIP"]);

	CreatePoints({ client_ports, server_ports, client_ips });
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::CreatePoints(vector<vector<string>> data_)
{
	for (int i = 0; i < (int)data_[0].size(); ++i)
	{
		vecPoints.push_back(make_shared<Point>(data_[0][i], data_[1][i], data_[2][i], CFG::data[CFG::outerIP]));
	}
	for (int i = 1; i < (int)vecPoints.size(); ++i)
	{
		vecPoints[i]->socket->ChangeIO(vecPoints[0]->socket->io);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
vector<string> Cnf::ParseLine(string line_)
{
	vector<string> result;
	size_t fd = 0;
	while (fd != string::npos)
	{
		fd = line_.find(" ");
		result.push_back(line_.substr(0, fd));

		if (fd == string::npos) break;
		line_ = line_.substr(fd + 1);
	}
	return result;
}