// Testing.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TestClass.h"

void testfunc1(string s_);
void testfunc2(int i_);
void testfunc3(boost::any a_);

void TestModul();
void TestMapFunc();
void TestMapEraseEmpty();
int main()
{
	
	TestMapEraseEmpty();
	system("pause");
	return 0;
}

void TestMapEraseEmpty()
{
	map<string, string> mapStr;
	mapStr["q1"] = "q1-1";
	for (auto&e : mapStr)
	{
		cout << "\nname=" << e.first << " value=" << e.second;
	}
	mapStr.erase("q2");
	for (auto&e : mapStr)
	{
		cout << "\nname=" << e.first << " value=" << e.second;
	}
}
void TestModul()
{
	Test1 t1;
	Test2 t2;
	TESTDATA td;
	SHP_TD shp_td = make_shared<TESTDATA>();

	HOLDER::AddModul("Test1", t1, t1.Register());
	HOLDER::AddModul("Test2", t2, t2.Register());

	HOLDER::Execute("Test1", "first hi!", 0, shp_td);
}
void TestMapFunc()
{
	string s = "qqq";
	int i = 1;
	map<string, boost::function<void(boost::any)>> mapModulFunc;
	vector<string> badNames;
	mapModulFunc["any"] = boost::bind(&testfunc3, _1);
	mapModulFunc["qqq"];
	mapModulFunc["qqqq"];
	mapModulFunc["qaq"];
	for (auto&e : mapModulFunc)
	{
		cout << "\nName=" << e.first;
		try
		{
			e.second(i);
			e.second(s);
		}
		catch (...)
		{
			cout << " Doest not exists";
			badNames.push_back(e.first);
			//mapModulFunc.erase(e.first);
		}
	}
	for (auto& e : badNames)
	{
		mapModulFunc.erase(e);
	}
	for (auto& e : badNames)
	{
		mapModulFunc.erase(e);
	}
	cout << "\nSecond round";
	for (auto&e : mapModulFunc)
	{
		cout << "\nName=" << e.first;
		try
		{
			e.second(i);
			e.second(s);
		}
		catch (...)
		{
			cout << " Doest not exists";
			mapModulFunc.erase(e.first);
		}
	}
}
void testfunc1(string s_)
{
	cout << "\nString:" << s_;
}
void testfunc2(int i_)
{
	cout << "\nint:" << i_;
}
void testfunc3(boost::any a_)
{
	try
	{
		string s = boost::any_cast<string>(a_);
		cout << "\nstring:" << s;
	}
	catch (...)
	{
		int i = boost::any_cast<int>(a_);
		cout << "\nint:" << i;
	}
}