#pragma once
#include "stdafx.h"

struct RTP
{
	/* первый байт */
	uint8_t csrc_len : 4;//
	uint8_t extension : 1;//
	uint8_t padding : 1;//
	uint8_t version : 2;//
	/* второй байт */
	uint8_t payload_type : 7;//
	uint8_t marker : 1;//
	/* третий-четвертый байты */
	uint16_t seq_no;
	/* пятый-восьмой байты */
	uint32_t timestamp;//
	/* девятый-двенадцатый байт */
	uint32_t ssrc;//
};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class RTP_struct
{
public:

	RTP_struct();

	RTP* Get(int);
	RTP* Get();

private:

	void Config();

	RTP header;
	long TS;
	int amount;

};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class TIME2
{
public:
	TIME2();

	boost::posix_time::ptime GetLast();
	boost::posix_time::ptime GetStart();
	boost::posix_time::time_duration GetFullDuration();
	boost::posix_time::time_duration GetActiveDuration();
	boost::posix_time::time_duration GetInactiveTime();

	void Update();

private:

	boost::posix_time::ptime start;
	boost::posix_time::ptime last;

};
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
class EVENTID
{
public:

	static string ReserveEventID();
	static void FreeEventID(string);

private:

	static vector<int> usedEventIDs;

};