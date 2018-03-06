#include "stdafx.h"
#include "Config.h"
#include "BasicStructs.h"

class SSTORAGE
{
public:
	//void Init();
	static SHP_SOCK GetSocket(string);
	static SHP_SOCK GetSocket(string, SHP_IO);
	//static SHP_SOCK ChangeIO(string, string);

	static string ReservePort();
	static void FreePort(string);

	static string ReserveEventID();
	static void FreeEventID(string);

	static map<string, SHP_SOCK> mapSocks;
private:
	//static IO& GetInstanceIO(SHP_IO);
	//static void FreeSocket();

	static vector<int> usedPorts;
	static vector<int> usedEventIDs;

};