#include "stdafx.h"
#include "TEST_OverLoad.h"
using namespace OverLoad;

string TESTCLASS::modulNameThis = "TESTCLASS";
string TESTCLASS::modulNameTest = "OverLoad";
map<int, SHP_TO> TESTCLASS::mapObj;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
TO::TO(int id_)
{
	mapData["CallID"] = to_string(id_);
	mapData["ClientPort"] = "port200" + mapData["CallID"];
	mapData["ClientIP"] = "ip200." + mapData["CallID"];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_IPL TO::GetIPL()
{
	SHP_IPL ipl; ipl.reset(new IPL(mapData));
	return ipl;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTCLASS::STARTTEST()
{
	//Init();
	Start();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTCLASS::Proceed(boost::any param_)
{
	try
	{
		SHP_IPL ipl = boost::any_cast<SHP_IPL>(param_);
		if (ipl->Param("CMD") == "DelCaller") DelUser(ipl);
		else
		{
			cout << "\nError cmd:" << ipl->PrintAll();
		}
	}
	catch (...)
	{
		cout << "\n TESTCLASS::Proceed catch";
		system("pause");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTCLASS::Init()
{
	COM::RegModul(modulNameThis, boost::bind(&TESTCLASS::Proceed, _1));
	for (int i = 0; i < 50; ++i)
	{
		SHP_TO new_to; new_to.reset(new TO(i));
		mapObj[i] = new_to;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTCLASS::Start()
{
	int amount = 20000;
	cout << "\nTest OverLoad Start for " + to_string(amount) + " repeats (" << std::to_string(this_thread::get_id().hash()) + ")";
	for (int i = 0; i < amount; ++i)
	{
		std::default_random_engine generator(i);
		std::uniform_int_distribution<int> distribution(0, 1);
		int dice = distribution(generator);
		if (dice == 0) AddUser();
		else DelUser();
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTCLASS::AddUser()
{
	std::default_random_engine generator((unsigned)chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<int> distribution(0, 49);
	int dice = distribution(generator);
	SHP_IPL ipl = mapObj[dice]->GetIPL();

	ipl->SetParam("To", modulNameTest);
	ipl->SetParam("From", modulNameThis);
	ipl->SetParam("CMD", "AddCaller");
	ipl->SetParam("ReplyTo", modulNameThis);
	COM::SendModul(modulNameTest, ipl);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTCLASS::DelUser()
{
	std::default_random_engine generator((unsigned)chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<int> distribution(0, 49);
	int dice = distribution(generator);
	SHP_IPL ipl = mapObj[dice]->GetIPL();

	ipl->SetParam("To", modulNameTest);
	ipl->SetParam("From", modulNameThis);
	ipl->SetParam("CMD", "DelCaller");
	ipl->SetParam("ReplyTo", modulNameThis);
	COM::SendModul(modulNameTest, ipl);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTCLASS::DelUser(SHP_IPL ipl_)
{
	ipl_->SetParam("To", modulNameTest);
	ipl_->SetParam("From", modulNameThis);
	ipl_->SetParam("CMD", "DelCaller");
	COM::SendModul(modulNameTest, ipl_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------