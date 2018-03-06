#pragma once
#include "stdafx.h"
#include "Structs.h"
#include "Functions.h"
using namespace std;

extern SHP_IPar init_Params;
//extern src::severity_logger< logging::trivial::severity_level > lg;


class MediaFile
{
public:
	MediaFile(string);

	SHP_CAVPacket GetPacket(int);
	int Size();

	string fileName;
	string error = "";
private:
	int OpenFile();
	void CutPackets(vector<SHP_CAVPacket>);

	vector<SHP_CAVPacket> data;
};
typedef shared_ptr<MediaFile> SHP_MediaFile;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class MusicStore
{
public:
	SHP_MediaFile GetFile(string);
private:
	SHP_MediaFile OpenNewFile(string);
	vector<SHP_MediaFile> data;
};
typedef shared_ptr<MusicStore> SHP_MusicStore;