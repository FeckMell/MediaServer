#include "stdafx.h"
#include "MusicStore.h"
using namespace MusicStore;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_IO MainFrame::ioMainFrame;
SHP_Socket MainFrame::fakeSocket;
SHP_thread MainFrame::receiveThread;
map<string, SHP_MusicFile> MainFrame::mapMusicFile;
string MainFrame::modulName = "MusicStore";
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::Init()
{
	COM::RegModul(modulName, boost::bind(&MainFrame::Proceed, _1));
	LOG::AddLogInstance(modulName);

	ioMainFrame.reset(new IO());
	fakeSocket.reset(new Socket(
		CFG::Param("OuterFakeIP"), CFG::Param("OuterFakePort"),
		CFG::Param("MyFakeIP"), CFG::Param("MyFakePort"), ioMainFrame));
	fakeSocket->AsyncReceiveFrom(boost::bind(&MainFrame::FakeReceive, _1, _2));
	receiveThread.reset(new thread([&](){ioMainFrame->run(); cout << "\nMusicStore::ThreadExit"; }));

	LOG::Log("info", modulName, "MusicStore::MainFrame::Init Done.");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::DeInit()
{

}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
/*
Nessesary IPL params for income:From To ReplyTo CMD FileName
Nessesary IPL params for responce: From To CMD File 
*/
void MainFrame::Proceed(boost::any param_)
{
	try
	{
		SHP_IPL ipl = boost::any_cast<SHP_IPL>(param_);
		if (ipl->Param("CMD") == "GetFile")      ioMainFrame->post(boost::bind(&MainFrame::GetFile, ipl));
		else if (ipl->Param("CMD") == "DelFile") ioMainFrame->post(boost::bind(&MainFrame::DelFile, ipl));
		else if (ipl->Param("CMD") == "CheckFile") CheckFile(ipl); // in this thread
		else LOG::Log("fatal", "Errors", "MusicStore::MainFrame::Proceed Error 1.");
	}
	catch (...){ LOG::Log("fatal", "Errors", "MusicStore::MainFrame::Proceed Error 2."); }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::GetFile(SHP_IPL ipl_)
{
	SHP_MusicFile file = nullptr;
	auto it = mapMusicFile.find(ipl_->Param("FileName"));
	if (it == mapMusicFile.end())
	{
		file = OpenNewFile(ipl_);
		if (file != nullptr) 
		{ 
			mapMusicFile.insert({ ipl_->Param("FileName"), file });
		}
		else
		{
			LOG::Log("fatal", "Errors", "MusicStore::MainFrame::GetFile not opened. FileName=" + ipl_->Param("FileName"));
		}
	}
	else
	{
		file = it->second;
	}

	SHP_IPL response = BasicIPL();
	response->SetParam("To", ipl_->Param("ReplyTo"));
	response->SetParam("CMD", "AddFile");
	if (file == nullptr) response->SetData("File", nullptr);
	else response->SetData("File", file->Data());
	COM::SendModul(ipl_->Param("ReplyTo"), response);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::DelFile(SHP_IPL ipl_)
{
	auto it = mapMusicFile.find(ipl_->Param("FileName"));
	if (it == mapMusicFile.end())
	{
		LOG::Log("fatal", "Errors", "MusicStore::MainFrame::DelFile not found. FileName=" + ipl_->Param("FileName"));
	}
	else
	{
		mapMusicFile.erase(ipl_->Param("FileName"));
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::CheckFile(SHP_IPL ipl_)
{
	ifstream file(CFG::Param("MediaPath") + CFG::Param("Slash") + ipl_->Param("FileName"));
	if (file.is_open()) { file.close(); ipl_->SetParam("FileExistance", "yes"); }
	else{ ipl_->SetParam("FileExistance", "no"); }
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_MusicFile MainFrame::OpenNewFile(SHP_IPL ipl_)
{
	SHP_MusicFile new_file; new_file.reset(new MusicFile(ipl_));
	if (new_file->Param("Error") == "") return new_file;
	else return nullptr;
}

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_IPL MainFrame::BasicIPL()
{
	map<string, string> map_data;
	map_data.insert({ "From", modulName });

	SHP_IPL ipl; ipl.reset(new IPL(map_data));
	return ipl;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MainFrame::FakeReceive(boost::system::error_code ec_, size_t size_)
{
	if (!ec_)
	{
		LOG::Log("fatal", "Errors", "MusicStore::MainFrame::FakeReceive received");
		fakeSocket->AsyncReceiveFrom(boost::bind(&MainFrame::FakeReceive, _1, _2));
	}
	else
	{
		LOG::Log("fatal", "Errors", "MusicStore::MainFrame::FakeReceive else");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
