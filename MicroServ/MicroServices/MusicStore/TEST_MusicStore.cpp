#include "stdafx.h"
#include "TEST_MusicStore.h"
using namespace MusicStore;

//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
TO::TO(string filename_)
{
	fileName = filename_;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TO::GetFile()
{
	SHP_IPL ipl = BasicIPL();
	ipl->SetParam("CMD", "GetFile");
	COM::SendModul("MusicStore", ipl);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TO::DelFile()
{
	SHP_IPL ipl = BasicIPL();
	ipl->SetParam("CMD", "DelFile");
	COM::SendModul("MusicStore", ipl);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TO::CheckFile()
{
	SHP_IPL ipl = BasicIPL();
	ipl->SetParam("CMD", "CheckFile");
	COM::SendModul("MusicStore", ipl);
	LOG::Log("fatal", "TESTMUSICSTORE", "CheckFile result =" + ipl->Param("FileExistance"));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_IPL TO::BasicIPL()
{
	map<string, string> params;
	params.insert({ "From", "TESTMUSICSTORE" });
	params.insert({ "To", "MusicStore" });
	params.insert({ "ReplyTo", "TESTMUSICSTORE" });
	params.insert({ "FileName", fileName });
	SHP_IPL ipl; ipl.reset(new IPL(params));
	return ipl;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_IO TESTMUSICSTORE::ioThis;
SHP_Socket TESTMUSICSTORE::fakeSocket;
SHP_thread TESTMUSICSTORE::receiveThread;

string TESTMUSICSTORE::modulNameThis = "TESTMUSICSTORE";
string TESTMUSICSTORE::modulNameTest = "MusicStore";
map<int, SHP_TO> TESTMUSICSTORE::mapTO;
map<string, string> TESTMUSICSTORE::mapData;
vector<SHP_VecSockBuf> TESTMUSICSTORE::vecSavedFiles;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTMUSICSTORE::Init(map<string, string> params_)
{
	mapData = params_;

	MainFrame::Init();

	InitUsual();
	InitTO();
	vecSavedFiles.push_back(nullptr);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTMUSICSTORE::InitUsual()
{
	COM::RegModul(modulNameThis, boost::bind(&TESTMUSICSTORE::Proceed, _1));
	LOG::AddLogInstance(modulNameThis);

	ioThis.reset(new IO());
	fakeSocket.reset(new Socket(
		CFG::Param("OuterFakeIP"), CFG::Param("OuterFakePort"),
		CFG::Param("MyFakeIP"), CFG::Param("MyFakePort"), ioThis));
	fakeSocket->AsyncReceiveFrom(boost::bind(&TESTMUSICSTORE::FakeReceive, _1, _2));
	receiveThread.reset(new thread([&](){ioThis->run(); cout << "\nTESTMUSICSTORE::ThreadExit"; }));

	LOG::Log("info", modulNameThis, "TESTMUSICSTORE::Init Done.");
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTMUSICSTORE::InitTO()
{
	for (int i = 0; i < stoi(mapData["FilesNum"]); ++i)
	{
		SHP_TO new_to; new_to.reset(new TO(mapData["FileNamePrefix"] + to_string(i) + mapData["FileNameSuffix"]));
		mapTO.insert({ i, new_to });
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTMUSICSTORE::Start()
{
	vector<SHP_thread> vec_threads;

	int threads_amount = stoi(mapData["ThreadsNum"]);	

	for (int i = 0; i < threads_amount; ++i)
	{
		SHP_thread th; th.reset(new thread([]{TESTMUSICSTORE::StartThread(); }));
		vec_threads.push_back(th);
	}
	for (int i = 0; i < threads_amount; ++i)
	{
		vec_threads[i]->join();
	}
	cout << "\nTESTMUSICSTORE Done";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTMUSICSTORE::StartThread()
{
	int test_size = stoi(mapData["TestSize"]);

	for (int i = 0; i < test_size; ++i)
	{
		this_thread::sleep_for(chrono::milliseconds(Random(i, 5, 20)));
		for (auto& e : mapTO)
		{
			this_thread::sleep_for(chrono::milliseconds(Random(i, 5, 17)));
			int random = Random(i, 0, 2);
			switch (random)
			{
				case 0:
					e.second->GetFile();
					break;
				case 1:
					e.second->DelFile();
					break;
				case 2:
					e.second->CheckFile();
					break;
			}
		}
	}
	mapTO[1]->DelFile();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTMUSICSTORE::Proceed(boost::any param_)
{
	ioThis->post(boost::bind(&TESTMUSICSTORE::ProceedResponse, param_));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTMUSICSTORE::ProceedResponse(boost::any param_)
{
	try
	{
		SHP_IPL ipl = boost::any_cast<SHP_IPL>(param_);
		if (ipl->Param("CMD") == "AddFile") 
		{
			SaveFile(ipl);
		}
		else
		{
			cout << "\nError cmd:" << ipl->PrintAll();
			system("pause");
		}
	}
	catch (...)
	{
		cout << "\n TESTMUSICSTORE::Proceed catch";
		system("pause");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int TESTMUSICSTORE::Random(int extra_seed_, int begin_, int end_)
{
	std::default_random_engine generator((unsigned)(chrono::system_clock::now().time_since_epoch().count()+ extra_seed_));
	std::uniform_int_distribution<int> distribution(begin_, end_);
	return distribution(generator);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTMUSICSTORE::CheckSavedFiles()
{
	cout << "\nChecking files: filesnum=" << vecSavedFiles.size();
	for (int i = 0; i < (int)vecSavedFiles.size(); ++i)
	{
		if (vecSavedFiles[i] != nullptr) cout << "\ni=" << i << " size=" << vecSavedFiles[i]->size();
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTMUSICSTORE::SaveFile(SHP_IPL ipl_)
{
	if (ipl_->Param("Data") == "")
	{
		cout << "\nTESTMUSICSTORE::SaveFile bad data name";
		system("pause");
	}
	try
	{
		auto file = boost::any_cast<SHP_VecSockBuf>(ipl_->GetData(ipl_->Param("Data")));
		//if (file != nullptr) vecSavedFiles.push_back(file);
		//cout << "\nSize=" << vecSavedFiles.size();
		vecSavedFiles[0] = file;
	}
	catch (...)
	{
		try
		{
			auto file = boost::any_cast<nullptr_t>(ipl_->GetData(ipl_->Param("Data")));
			LOG::Log("fatal", "TESTMUSICSTORE", "SaveFile nullptr_t");
		}
		catch (...)
		{
			cout << "\nTESTMUSICSTORE::SaveFile bad file type";
			system("pause");
		}
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void TESTMUSICSTORE::FakeReceive(boost::system::error_code ec_, size_t size_)
{
	if (!ec_)
	{
		LOG::Log("fatal", "Errors", "TESTMUSICSTORE::FakeReceive received");
		fakeSocket->AsyncReceiveFrom(boost::bind(&TESTMUSICSTORE::FakeReceive, _1, _2));
	}
	else
	{
		LOG::Log("fatal", "Errors", "TESTMUSICSTORE::FakeReceive else");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------