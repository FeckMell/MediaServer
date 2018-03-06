#include "stdafx.h"
#include "MusicStore_MusicFile.h"
using namespace MusicStore;

MusicFile::MusicFile(SHP_IPL ipl_)
{
	mapData["FileName"] = ipl_->Param("FileName");
	if (OpenFile() != 0)
	{
		mapData["Error"] = "error read file";
		LOG::Log("fatal", "Errors", "MusicStore::MusicFile::MusicFile error");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MusicFile::Param(string name_)
{
	return mapData[name_];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_VecSockBuf MusicFile::Data()
{
	return dataThis;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int MusicFile::OpenFile()
{
	int action_result = 0;
	string file = CFG::Param("MediaPath") + CFG::Param("Slash") + Param("FileName");
	AVFormatContext* ifcx = nullptr;
	vector<SHP_PACKET> vec_packet;
	vector<SHP_SockBuf> vec_file;

	action_result = avformat_open_input(&ifcx, file.c_str(), 0, 0);
	if (action_result < 0) return -1;
	while (action_result >= 0)
	{
		SHP_PACKET new_packet; new_packet.reset(new PACKET());// get rid of it
		av_read_frame(ifcx, new_packet->Get());
		vec_packet.push_back(new_packet);
		action_result = new_packet->Size() - 1;//avreadframe does not return -1 for some reason
		
	}
	avformat_close_input(&ifcx);

	for (auto&e : vec_packet)
	{
		vector<SHP_SockBuf> temp_vec = SockBuf::CutData(e->Data(), e->Size(), 80);
		vec_file.insert(vec_file.end(), temp_vec.begin(), temp_vec.end());
		if (vec_file.back()->Size() != 80) vec_file.pop_back();
	}
	dataThis = make_shared<vector<SHP_SockBuf>>(vec_file);
	dataThis->shrink_to_fit();

	return 0;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------