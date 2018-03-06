#pragma once
#include "stdafx.h"
#include "Functions.h"
extern string DateStr;
extern std::string PathEXE;

class Logger
{
public:
	Logger(){ std::cout << "\nLog class created. " + PathEXE; }
	void Create(){ std::cout << "\nLog class created. " + PathEXE; OpenLogFiles(); }
	void AddToLog(unsigned type, std::string text);
	void Run();

	void output(std::string text, int i);
private:
	//method
	void OpenLogFiles();
	//void output(std::string text, int i);
	std::string pop(int i);
	void Clean();
	void reinit();

	//data
	std::vector<std::queue<std::string>> buffer;
	std::vector<std::ofstream> file;

	std::mutex  mutex_;
};