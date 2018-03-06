#include "stdafx.h"
#include "Cnf.h"

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
Cnf::Cnf(SHP_IPL ipl_)
{
	cnfID = ipl_->data[IPL::eventID];
	ParsePoints(ipl_);
	mixerAudio.reset(new Audio(vecPoints));
}
Cnf::~Cnf()
{
	vecPoints.clear(); 
	mixerAudio.reset();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::MD(SHP_IPL ipl_)
{
	mixerAudio->Stop();
	AddRmPoint({
		ipl_->data[IPL::clientPort],
		ipl_->data[IPL::serverPort],
		ipl_->data[IPL::clientIP]
	});
	mixerAudio->MD(vecPoints);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::DL(SHP_IPL ipl_)
{
	mixerAudio->Stop();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::ParsePoints(SHP_IPL ipl_)
{
	vector<vector<string>> points_params;
	points_params.resize(3);

	string client_ports = ipl_->data[IPL::clientPort];
	string server_ports = ipl_->data[IPL::serverPort];
	string client_IPs = ipl_->data[IPL::clientIP];
	size_t fd=0;
	while (fd != string::npos)
	{
		fd = client_ports.find(" ");
		points_params[0].push_back(client_ports.substr(0, fd));
		if (fd == string::npos) break;
		client_ports = client_ports.substr(fd+1);
	}fd = 0;
	while (fd != string::npos)
	{
		fd = server_ports.find(" ");
		points_params[1].push_back(server_ports.substr(0, fd));
		if (fd == string::npos) break;
		server_ports = server_ports.substr(fd+1);
	}fd = 0;
	while (fd != string::npos)
	{
		fd = client_IPs.find(" ");
		points_params[2].push_back(client_IPs.substr(0, fd));
		if (fd == string::npos) break;
		client_IPs = client_IPs.substr(fd+1);
	}fd = 0;

	CreatePoints(points_params);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::CreatePoints(vector<vector<string>> data_)
{
	for (int i = 0; i < (int)data_[0].size(); ++i)
		vecPoints.push_back(
		make_shared<CnfPoint>(
		data_[0][i], 
		data_[1][i], 
		data_[2][i], 
		init_Params->data[IPar::outerIP],
		ioCnf
		));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Cnf::AddRmPoint(vector<string> params_)
{
	for (int i = 0; i < (int)vecPoints.size(); ++i)
	{
		if (vecPoints[i]->serverPort == params_[1])
		{
			vecPoints.erase(remove(vecPoints.begin(), vecPoints.end(), vecPoints[i]), vecPoints.end());
			return;
		}
	}
	vecPoints.push_back(
		make_shared<CnfPoint>(
		params_[0],
		params_[1],
		params_[2],
		init_Params->data[IPar::outerIP],
		ioCnf
		));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------