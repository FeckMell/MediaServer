#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
#include "Cnf.h"
using namespace std;

extern SHP_IPar init_Params;
//extern boost::asio::io_service io_Server;
extern boost::asio::io_service io_Apps;
//extern SHP_Socket outer_Socket;
extern SHP_Socket inner_Socket;

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class Control
{
public:
	Control();
	void Preprocessing(SHP_IPL);


private:
	void CR(SHP_IPL);
	void MD(SHP_IPL);
	void DL(SHP_IPL);

	SHP_Cnf FindCnf(SHP_IPL);
	void RemoveCnf(SHP_Cnf);

	vector<SHP_Cnf> vecCnf;
};
typedef shared_ptr<Control> SHP_Control;