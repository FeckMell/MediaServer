#pragma once
#include "stdafx.h"
#include "Functions.h"
extern string DateStr;
extern std::string PathEXE;

class Logger
{
public:
	//Logger(){ std::cout << "\nLog class created. " + PathEXE; OpenLogFiles(); }
	void Create(){ std::cout << "\nLog class created. " + PathEXE; OpenLogFiles(); }
	void AddToLog(unsigned type, std::string text);
	void Run();
private:
	//method
	void OpenLogFiles();
	void output(std::string text, int i);
	std::string pop(int i, int j);
	void Clean();
	void reinit();

	//data
	std::vector<std::vector<std::string>> buffer;
	std::vector<HANDLE> file;
	std::vector<DWORD> filesize;

	std::mutex  mutex_;

};