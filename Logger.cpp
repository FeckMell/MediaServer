#include "stdafx.h"
#include "Logger.h"

void Logger::OpenLogFiles()
{
	using namespace boost;
	reinit();
	buffer.resize(file2.size());
}
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
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
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
void Logger::output(std::string text, int i)
{
	file2[i].write(text.c_str(),text.size());
	file2[i].flush();
}
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
std::string Logger::pop(int i)
{
	std::string text;
	text = buffer[i].front();//
	mutex_.lock();
	buffer[i].pop();
	mutex_.unlock();
	return text;
}
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
void Logger::AddToLog(unsigned type, std::string text)
{
	if (buffer.size() < type){ std::cout << "size!"; return; }
	mutex_.lock();
	buffer[type].push(text);
	mutex_.unlock();
}
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
void Logger::Clean()
{
	for (auto&e : file2)
	{
		e.close();
	}
	file2.clear();
}
//-*/----------------------------------------------------------
//-*/----------------------------------------------------------
void Logger::reinit()
{
	using namespace boost;
	DateStr = GetDate();
	std::string tempPath = PathEXE +"Logs\\"+ DateStr + "_";

	std::vector<std::string> path;
	/*0*/path.push_back(tempPath + "LOGS_main.txt");
	/*1*/path.push_back(tempPath + "LOGS_ConfRoom.txt");
	/*2*/path.push_back(tempPath + "LOGS_ConfAudio.txt");
	/*3*/path.push_back(tempPath + "LOGS_Server.txt");
	/*4*/path.push_back(tempPath + "LOGS_FilterInit.txt");
	/*5*/path.push_back(tempPath + "LOGS_Ann.txt");
	/*6*/path.push_back(tempPath + "LOGS_MGCPControl.txt");
	/*7*/path.push_back(tempPath + "LOGS_Proxy.txt");
	/*8*/path.push_back(tempPath + "LOGS_ConfPoint.txt");
	/*9*/path.push_back(tempPath + "LOGS_SIPControl.txt");
	file2.resize(path.size());
	for (unsigned i = 0; i < path.size(); ++i)
	{
		file2[i].open(path[i], std::ofstream::app);
		time_t rawtime;
		struct tm * t;
		time(&rawtime);
		t = localtime(&rawtime);
		std::string time = "";
		std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
		time += DateStr + "/" + to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + "/" + to_string(t1.time_since_epoch().count() % 1000);
		output("\nLogs created on: " + time, i);
	}
}