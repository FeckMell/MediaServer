#include "stdafx.h"
#include "MusicStore.h"
using namespace ann;


MediaFile::MediaFile(string filename_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), debug) << "MediaFile::MediaFile(...): filename=" << filename_;
	fileName = filename_;
	if (OpenFile() == -1) 
	{ 
		BOOST_LOG_SEV(LOG::GL(LOG::L::ann), fatal) << "MediaFile::MediaFile(...): error read file";
		error = "error read file"; 
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int MediaFile::OpenFile()
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "MediaFile::OpenFile()";
	int action_result = 0;

	//string file = init_Params->data[STARTUP::mediaPath] + "\\" + fileName;
	string file = CFG::data[CFG::mediaPath] + "\\" + fileName;
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), debug) << "MediaFile::OpenFile(): filePath=" << file;
	AVFormatContext* ifcx = nullptr;
	action_result = avformat_open_input(&ifcx, file.c_str(), 0, 0);
	if (action_result < 0) return -1;

	vector<SHP_PACKET> readed_file;
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), debug) << "MediaFile::OpenFile(): read file in while";
	while (action_result >= 0)
	{
		SHP_PACKET readed_packet = make_shared<PACKET>(0);
		av_read_frame(ifcx, readed_packet->Get());
		readed_file.push_back(readed_packet);
		action_result = readed_packet->Size() - 1;//avreadframe does not return -1 for some reason
	}
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "MediaFile::OpenFile(): CutPackets(readed_file);";
	CutPackets(readed_file);
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "MediaFile::OpenFile(): CutPackets(readed_file);->avformat_close_input(&ifcx);";
	avformat_close_input(&ifcx);
	data.shrink_to_fit();
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "MediaFile::OpenFile() DONE";
	return 0;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MediaFile::CutPackets(vector<SHP_PACKET> file_)
{
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "MediaFile::CutPackets(...)";
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
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "MediaFile::CutPackets(...) DONE";
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
	BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "MusicStore::OpenNewFile(...)";
	SHP_MediaFile new_file = make_shared<MediaFile>(filename_);
	if (new_file->error == "")
	{
		data.push_back(new_file);
		BOOST_LOG_SEV(LOG::GL(LOG::L::ann), trace) << "MusicStore::OpenNewFile(...): DONE";
		return new_file;
	}
	else return nullptr;
}