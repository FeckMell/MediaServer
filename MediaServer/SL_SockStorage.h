#include "stdafx.h"
#include "SL_Config.h"
#include "SL_BasicStructs.h"

class SSTORAGE
{
public:

	static SHP_SOCK GetSocket(string);
	//static SHP_SOCK GetSocket(string, SHP_IO);

	static string ReservePort();
	static void FreePort(string);

	static string ReserveEventID();
	static void FreeEventID(string);

	static map<string, SHP_SOCK> mapSocks;

private:

	static vector<int> usedPorts;
	static vector<int> usedEventIDs;

};