#include "stdafx.h"
#include "MScnf_Cnf.h"
using namespace cnf;


Cnf::Cnf(SHP_IPL ipl_)
{
	
	cnfID = ipl_->data["EventID"];
	
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
		ipl_->data["ClientPort"],
		ipl_->data["ServerPort"],
		ipl_->data["ClientIP"]
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

	string client_ports = ipl_->data["ClientPort"];
	string server_ports = ipl_->data["ServerPort"];
	string client_IPs = ipl_->data["ClientIP"];
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
	{
		vecPoints.push_back(make_shared<CnfPoint>(data_[0][i], data_[1][i], data_[2][i], CFG::data[CFG::outerIP]));
	}
	for (int i = 1; i < (int)vecPoints.size(); ++i)
	{
		vecPoints[i]->socket->ChangeIO(vecPoints[0]->socket->io);
	}
	
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
		CFG::data[CFG::outerIP]
		));
	vecPoints.back()->socket->ChangeIO(vecPoints[0]->socket->io);
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------