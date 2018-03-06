#include "stdafx.h"
#include "FAnn_Music.h"
using namespace NFAnn;
vector<SHP_MusicFile> MusicStore::vecFile = {};

MusicFile::MusicFile(string filename_)
{
	mapData["FileName"] = filename_;
	if (OpenFile() == -1)
	{
		mapData["Error"]= "error read file";
		LOG::Log("fatal", "Errors", "MusicFile::MusicFile error");
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int MusicFile::OpenFile()
{

	int action_result = 0;

	string file = CFG::Param("MediaPath") + CFG::Param("Slash") + Param("FileName");

	AVFormatContext* ifcx = nullptr;
	action_result = avformat_open_input(&ifcx, file.c_str(), 0, 0);
	if (action_result < 0) return -1;

	vector<SHP_PACKET> readed_file;

	while (action_result >= 0)
	{
		SHP_PACKET readed_packet; readed_packet.reset(new PACKET(0));// get rid of it
		av_read_frame(ifcx, readed_packet->Get());
		readed_file.push_back(readed_packet);
		action_result = readed_packet->Size() - 1;//avreadframe does not return -1 for some reason
	}

	CutPackets(readed_file);

	avformat_close_input(&ifcx);
	vecPackets.shrink_to_fit();

	return 0;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MusicFile::CutPackets(vector<SHP_PACKET> file_)
{
	for (int i = 0; i < (int)file_.size(); ++i)
	{
		int j = 0;
		while ((j + 1) * 160 <= file_[i]->Size())
		{
			SHP_PACKET cut_packet = FFF::CreatePacket(file_[i]->Data() + j * 160, 160);
			vecPackets.push_back(cut_packet);
			++j;
		}
	}//last bit is thrown as it less than 20ms. No one will notice. Maybe.
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int MusicFile::Size()
{
	return vecPackets.size();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_PACKET MusicFile::GetPacket(int num_)
{
	if (num_ >= (int)vecPackets.size() || num_ < 0) return nullptr;
	else return vecPackets[num_];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string MusicFile::Param(string name_)
{
	return mapData[name_];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_MusicFile MusicStore::GetFile(string filename_)
{
	for (auto& e : vecFile)
	{
		if (e->Param("FileName") == filename_) return e;
	}
	return OpenNewFile(filename_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_MusicFile MusicStore::OpenNewFile(string filename_)
{
	SHP_MusicFile new_file; new_file.reset(new MusicFile(filename_));
	if (new_file->Param("Error") == "")
	{
		vecFile.push_back(new_file); //single file DEBUG
		return new_file;
	}
	else return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
