// Testing.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TestClass.h"


int main()
{
	Test1 t1;
	Test2 t2;
	TESTDATA td;
	SHP_TD shp_td = make_shared<TESTDATA>();

	HOLDER::AddModul("Test1", t1, t1.Register());
	HOLDER::AddModul("Test2", t2, t2.Register());

	HOLDER::Execute("Test1", "first hi!", 0, shp_td);

	system("pause");
	return 0;
}

