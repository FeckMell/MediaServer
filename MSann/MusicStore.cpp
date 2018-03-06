#include "MusicStore.h"

MediaFile::MediaFile(string filename_)
{
	BOOST_LOG_SEV(lg, debug) << "MediaFile::MediaFile(...): filename=" << filename_;
	fileName = filename_;
	if (OpenFile() == -1) 
	{ 
		BOOST_LOG_SEV(lg, fatal) << "MediaFile::MediaFile(...): error read file";
		error = "error read file"; 
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int MediaFile::OpenFile()
{
	BOOST_LOG_SEV(lg, trace) << "MediaFile::OpenFile()";
	int action_result = 0;

	string file = init_Params->data[IPar::mediaPath] + "\\" + fileName;
	BOOST_LOG_SEV(lg, debug) << "MediaFile::OpenFile(): filePath=" << file;
	AVFormatContext* ifcx = nullptr;
	action_result = avformat_open_input(&ifcx, file.c_str(), 0, 0);
	if (action_result < 0) return -1;

	vector<SHP_CAVPacket> readed_file;
	BOOST_LOG_SEV(lg, debug) << "MediaFile::OpenFile(): read file in while";
	while (action_result >= 0)
	{
		SHP_CAVPacket readed_packet = make_shared<CAVPacket>(0);
		av_read_frame(ifcx, readed_packet->Get());
		readed_file.push_back(readed_packet);
		action_result = readed_packet->Size() - 1;//avreadframe does not return -1 for some reason
	}
	BOOST_LOG_SEV(lg, trace) << "MediaFile::OpenFile(): CutPackets(readed_file);";
	CutPackets(readed_file);
	BOOST_LOG_SEV(lg, trace) << "MediaFile::OpenFile(): CutPackets(readed_file);->avformat_close_input(&ifcx);";
	avformat_close_input(&ifcx);
	BOOST_LOG_SEV(lg, trace) << "MediaFile::OpenFile() DONE";
	return 0;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void MediaFile::CutPackets(vector<SHP_CAVPacket> file_)
{
	BOOST_LOG_SEV(lg, trace) << "MediaFile::CutPackets(...)";
	for (int i = 0; i < (int)file_.size(); ++i)
	{
		int j = 0;
		while ((j + 1) * 160 <= file_[i]->Size())
		{
			SHP_CAVPacket cut_packet = make_shared<CAVPacket>(160);
			memcpy(cut_packet->Data(), file_[i]->Data() + j * 160, 160);
			data.push_back(cut_packet);
			++j;
		}
	}//last bit is thrown as it less than 20ms. No one will notice. Maybe.
	BOOST_LOG_SEV(lg, trace) << "MediaFile::CutPackets(...) DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int MediaFile::Size()
{
	return data.size();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_CAVPacket MediaFile::GetPacket(int num_)
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
	BOOST_LOG_SEV(lg, trace) << "MusicStore::OpenNewFile(...)";
	SHP_MediaFile new_file = make_shared<MediaFile>(filename_);
	if (new_file->error == "")
	{
		data.push_back(new_file);
		BOOST_LOG_SEV(lg, trace) << "MusicStore::OpenNewFile(...): DONE";
		return new_file;
	}
	else return nullptr;
}