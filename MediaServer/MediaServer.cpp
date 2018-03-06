// MediaServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Structs.h"
#include <Windows.h>
#include <WinUser.h> //��� ����������� ����
#include <ShellAPI.h> //��� ������� ���������

SHP_IPar init_Params;
int _tmain(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");//Russia location
	cout << "\n���� MediaServer 2.0 (20.10.2016 / 12:08)\n";//Info about app.
	init_Params.reset(new IPar(argv[0])); //Create storage for init parametrs and parse init file
	//cout << init_Params->GetParams(); //show parametrs
	//system((init_Params->data[IPar::PathEXE] + "MSmgcp.exe").c_str());


	cout << init_Params->data[IPar::PathEXE] + "MSmgcp.exe";
	ShellExecute(0, NULL, (LPWSTR)(init_Params->data[IPar::PathEXE] + "MSmgcp").c_str(), NULL, NULL, SW_SHOWNORMAL);
	system("pause");
	return 0;
}

