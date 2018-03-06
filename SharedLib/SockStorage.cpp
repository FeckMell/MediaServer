#include "stdafx.h"
#include "SockStorage.h"
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
map<string, SHP_SOCK> SSTORAGE::mapSocks = {};

vector<int> SSTORAGE::usedPorts = {};
vector<int> SSTORAGE::usedEventIDs = {};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_SOCK SSTORAGE::GetSocket(string port_)
{
	SHP_SOCK found_sock = mapSocks[port_];
	if (found_sock != nullptr) return found_sock;
	else
	{
		SHP_IO new_io; new_io.reset(new IO());
		SHP_SOCK new_sock; new_sock.reset(new SOCK(CFG::data[CFG::outerIP], stoi(port_), new_io));

		mapSocks[port_] = new_sock;
		return new_sock;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string SSTORAGE::ReservePort()
{
	int free_port = stoi(CFG::data[CFG::rtpPort]);
	if (usedPorts.size() == 0)
	{
		usedPorts.push_back(free_port);
		return to_string(free_port);
	}
	for (unsigned i = 0; i < usedPorts.size(); ++i)
	{
		if (usedPorts[i] != free_port)
		{
			usedPorts.push_back(free_port);
			sort(usedPorts.begin(), usedPorts.end());
			return to_string(free_port);
		}
		free_port += 2;
	}
	usedPorts.push_back(free_port);
	sort(usedPorts.begin(), usedPorts.end());
	return to_string(free_port);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SSTORAGE::FreePort(string port_)
{
	usedPorts.erase(remove(usedPorts.begin(), usedPorts.end(), stoi(port_)), usedPorts.end());
	mapSocks.erase(port_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string SSTORAGE::ReserveEventID()
{
	int free_event_id = 0;
	if (usedEventIDs.size() == 0) { usedEventIDs.push_back(free_event_id);  return to_string(free_event_id); }
	for (unsigned i = 0; i < usedEventIDs.size(); ++i)
	{
		if (usedEventIDs[i] != free_event_id)
		{
			usedEventIDs.push_back(free_event_id);
			sort(usedEventIDs.begin(), usedEventIDs.end());
			return to_string(free_event_id);
		}
		free_event_id++;
	}
	usedEventIDs.push_back(free_event_id);
	sort(usedEventIDs.begin(), usedEventIDs.end());
	return to_string(free_event_id);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SSTORAGE::FreeEventID(string event_id_)
{
	usedEventIDs.erase(remove(usedEventIDs.begin(), usedEventIDs.end(), stoi(event_id_)), usedEventIDs.end());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
