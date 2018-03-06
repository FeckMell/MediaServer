// MediaServer.cpp : Defines the entry point for the console application.
//

#include "../SharedSource/stdafx.h"
#include "Structs.h"
#include "ModulControl.h"

SHP_STARTUP init_Params;

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");//Russia location
	cout << "\nÒÅÑÒ MediaServer 2.0 (20.10.2016 / 12:08)\n";//Info about app.
	init_Params.reset(new STARTUP(argv[0])); //Create storage for init parametrs and parse init file
	//cout<<init_Params->GetParams();
	MC controller;

	system("pause");
	return 0;
}
