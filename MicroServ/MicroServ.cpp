// MicroServ.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SharedLib.h"
#include "MicroServices\OverLoad\OverLoad.h"
#include "MicroServices\OverLoad\TEST_OverLoad.h"
#include "MicroServices\RTPReceiver\RTPReceiver.h"
#include "MicroServices\RTPReceiver\TEST_RTPReceiver.h"
#include "MicroServices\MusicStore\MusicStore.h"
#include "MicroServices\MusicStore\TEST_MusicStore.h"
#include "MicroServices\Receiver\Receiver.h"

void Work();
void Test();

int main(int argc, char* argv[])
{
	CFG::Init(argv[0]);
	LOG::Init();
	Work();
	//Test();
	return 0;
}
void Test()
{
	MusicStore::TESTMUSICSTORE::Init({
		{ "ThreadsNum", "10" }, { "FilesNum", "6" }, { "TestSize", "40" },
		{ "FileNamePrefix", "test_musicfile_ (" }, { "FileNameSuffix", ").wav" }

	});
	MusicStore::TESTMUSICSTORE::Start();
	MusicStore::TESTMUSICSTORE::CheckSavedFiles();
	system("pause");
}
void Work()
{
	OverLoad::MainFrame::Init();
	RTPReceiver::MainFrame::Init();
	MusicStore::MainFrame::Init();
	Receiver::MainFrame::Init();
	cout << "\nDone";
	system("pause");
}



