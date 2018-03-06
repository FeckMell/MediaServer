#include "stdafx.h"
#include "FAnn.h"
using namespace NFAnn;
SHP_IO FAnn::ioForAll = nullptr;
SHP_Socket FAnn::fakeSocket = nullptr;
SHP_thread FAnn::actionThread = nullptr;
map<string, SHP_Point> FAnn::mapPoint = {};
mutex FAnn::dataMutex;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void FAnn::Init()
{
	LOG::AddLogInstance("FAnn");
	actionThread.reset(new thread(&FAnn::SendAnnFunc));
	COM::RegModul("ann", boost::function<void(string)>(&FAnn::ProceedRequest, _1));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void FAnn::ProceedRequest(string request_)
{
	SHP_IPL ipl; ipl.reset(new IPL(request_));
	if (ipl->Param("CMD") == "CR") Create(ipl);
	else if (ipl->Param("CMD") == "DL") Delete(ipl);
	else
	{
		LOG::Log("fatal", "Errors", "FAnn::ProceedRequest undone CMD:\n" + request_);
		cout << "\nFAnn::ProceedRequest undone:\n" << request_;
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void FAnn::Create(SHP_IPL ipl_)
{
	SHP_MusicFile new_file = MusicStore::GetFile(ipl_->Param("FileName"));
	SHP_Point new_point = mapPoint[ipl_->Param("CallID")];
	if (new_point != nullptr)
	{
		LOG::Log("fatal", "Errors", "(caution)FAnn::Create exists CallID:"+ipl_->Param("CallID"));
		//return;
		dataMutex.lock();
		mapPoint.erase(ipl_->Param("CallID"));
		new_point.reset();//
		dataMutex.unlock();
		new_point.reset();
		LOG::Log("fatal", "Errors", "(caution done)FAnn::Create exists CallID:" + ipl_->Param("CallID"));
	}

	new_point.reset(new Point(ipl_, new_file));
	dataMutex.lock();
	mapPoint[ipl_->Param("CallID")] = new_point;
	dataMutex.unlock();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void FAnn::Delete(SHP_IPL ipl_)
{
	dataMutex.lock();
	mapPoint.erase(ipl_->Param("CallID"));
	dataMutex.unlock();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void FAnn::FakeReceive(boost::system::error_code ec_, size_t size_)
{
	if (!ec_)
	{
		LOG::Log("fatal", "Errors", "FAnn::FakeReceive");
		this_thread::sleep_for(chrono::milliseconds(5)); //DebugTest
		fakeSocket->AsyncReceiveFrom(boost::bind(&FAnn::FakeReceive, _1, _2));
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void FAnn::SendAnnFunc()
{
	while (true)
	{
		auto start_time = chrono::steady_clock::now();
		dataMutex.lock();
		for (auto& e : mapPoint)
		{
			e.second->SendNextPacket();
		}
		dataMutex.unlock();
		auto end_time = chrono::steady_clock::now();
		auto duration = chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
		this_thread::sleep_for(chrono::milliseconds(20 - duration.count()));
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------