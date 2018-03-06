#include "stdafx.h"
#include "Structs.h"

extern SHP_IPar init_Params;
using namespace std;

class MC
{
public:
	enum MODULES{ mgcp, ann, cnf, maxModules };
	MC();
private:
	bool RunProcess(string, DWORD, int);
	string MakeCmd();

	vector<STARTUPINFOA> si;
	vector<PROCESS_INFORMATION> pi;
};