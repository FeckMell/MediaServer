#include "stdafx.h"
#include "Logger.h"

void Logger::OpenLogFiles()
{
	using namespace boost;
	reinit();
	buffer.resize(file.size());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Logger::Run()
{
	while (true)
	{
		for (unsigned i = 0; i < buffer.size(); ++i)
		{
			int size = buffer[i].size();
			if (size>0)
			{
				for (int k = 0; k < size; ++k)
				{
					output(pop(i), i);
				}
			}
		}
		if (DateStr != GetDate())
		{
			Clean();
			reinit();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Logger::output(std::string text, int i)
{
	WriteFile(file[i], (text).c_str(), (text).size(), &filesize[i], NULL);
	filesize[i] += (text).size();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::string Logger::pop(int i)
{
	std::string text;
	mutex_.lock();
	text = buffer[i].front();//
	buffer[i].pop();
	mutex_.unlock();
	return text;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Logger::AddToLog(unsigned type, std::string text)
{
	if (buffer.size() < type){ std::cout << "size!"; return; }
	mutex_.lock();
	buffer[type].push(text);
	mutex_.unlock();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Logger::Clean()
{
	for (auto& e : file)
	{
		CloseHandle(e);
	}
	filesize.clear();
	file.clear();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Logger::reinit()
{
	using namespace boost;
	DateStr = GetDate();
	std::string tempPath = PathEXE + DateStr + "_";

	std::vector<std::string> path;
	/*0*/path.push_back(tempPath + "LOGS_main.txt");
	/*1*/path.push_back(tempPath + "LOGS_ConfRoom.txt");
	/*2*/path.push_back(tempPath + "LOGS_Mixer.txt");
	/*3*/path.push_back(tempPath + "LOGS_Server.txt");
	/*4*/path.push_back(tempPath + "LOGS_MixerInit.txt");
	/*5*/path.push_back(tempPath + "LOGS_Ann.txt");
	/*6*/path.push_back(tempPath + "LOGS_ConfControl.txt");
	/*7*/path.push_back(tempPath + "LOGS_Proxy.txt");
	for (unsigned i = 0; i < path.size(); ++i)
	{
		file.push_back(CreateFileA(
			path[i].c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
			);
		if (file[i] == INVALID_HANDLE_VALUE)
		{
			printf("ERROR %x \n", GetLastError());
			std::cout << "\nRELOAD NEEDED. LOG ERROR";//messagebox
		}
		filesize.push_back(0);
		time_t rawtime;
		struct tm * t;
		time(&rawtime);
		t = localtime(&rawtime);
		std::string time = "";
		std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
		time += DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
		output("Logs created on: " + time, i);
	}
}