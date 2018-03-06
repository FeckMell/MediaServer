#include "stdafx.h"
#include "MSann_MusicStore.h"
using namespace ann;


MediaFile::MediaFile(string filename_)
{
	fileName = filename_;
	if (OpenFile() == -1) 
	{ 
		error = "error read file"; 
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int MediaFile::OpenFile()
{
	
	int action_result = 0;

	string file = CFG::data[CFG::mediaPath] + "\\" + fileName;
	
	AVFormatContext* ifcx = nullptr;
	action_result = avformat_open_input(&ifcx, file.c_str(), 0, 0);
	if (action_result < 0) return -1;

	vector<SHP_PACKET> readed_file;
	
	while (action_result >= 0)
	{
		SHP_PACKET readed_packet = make_shared<PACKET>(0);
		av_read_frame(ifcx, readed_packet->Get());
		readed_file.push_back(readed_packet);
		action_result = readed_packet->Size() - 1;//avreadframe does not return -1 for some reason
	}
	
	CutPackets(readed_file);
	
	avformat_close_input(&ifcx);
	data.shrink_to_fit();
	
	return 0;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MediaFile::CutPackets(vector<SHP_PACKET> file_)
{
	for (int i = 0; i < (int)file_.size(); ++i)
	{
		int j = 0;
		while ((j + 1) * 160 <= file_[i]->Size())
		{
			SHP_PACKET cut_packet = make_shared<PACKET>(160);
			memcpy(cut_packet->Data(), file_[i]->Data() + j * 160, 160);
			data.push_back(cut_packet);
			++j;
		}
	}//last bit is thrown as it less than 20ms. No one will notice. Maybe.
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int MediaFile::Size()
{
	return data.size();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_PACKET MediaFile::GetPacket(int num_)
{
	if (num_ >= (int)data.size() || num_ < 0) return nullptr;
	else return data[num_];
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_MediaFile MusicStore::GetFile(string filename_)
{
	for (auto& e: data)
	{
		if (e->fileName == filename_) return e;
	}
	return OpenNewFile(filename_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_MediaFile MusicStore::OpenNewFile(string filename_)
{
	SHP_MediaFile new_file = make_shared<MediaFile>(filename_);
	if (new_file->error == "")
	{
		data.push_back(new_file);
		return new_file;
	}
	else return nullptr;
}