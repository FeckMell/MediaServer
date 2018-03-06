// MediaServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Structs.h"
#include "ModulControl.h"

SHP_IPar init_Params;
int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");//Russia location
	cout << "\nÒÅÑÒ MediaServer 2.0 (20.10.2016 / 12:08)\n";//Info about app.
	init_Params.reset(new IPar(argv[0])); //Create storage for init parametrs and parse init file
	//cout<<init_Params->GetParams();
	MC controller;

	system("pause");
	return 0;
}
/*
data[RTPPort] = "20000";
data[outerPort] = "2427";
data[innerPort] = "2427";
data[innerIP] = "127.0.0.1";
data[MaxTimeAnn] = "90";
data[MaxTimeCnf] = "1";
data[MaxTimePrx] = "1";
data[LogLevel] = "100";
data[MediaPath] = "MediaFiles";
*/
