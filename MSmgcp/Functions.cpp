#include "stdafx.h"
#include "Functions.h"
extern src::severity_logger< logging::trivial::severity_level > lg;
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//вырезает после начального слова
string get_substr(string target_, string aim_, string fin_)
{//target_ - откуда вырезаем, aim_ - начало, fin - конец
	auto fd_pos = target_.find(aim_);
	string result = "";
	if (fd_pos != string::npos)
		result = target_.substr(fd_pos + aim_.length(), target_.find(fin_, fd_pos + aim_.length()) - (fd_pos + aim_.length()));
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//вырезает полностью строку
string cut_substr(string target_, string aim_, string fin_)
{
	auto fd_pos = target_.find(aim_);
	string result = "";
	if (fd_pos != string::npos)
		result = target_.substr(fd_pos, target_.find(fin_, fd_pos + 1) - (fd_pos-1)); //(fd_pos-1) last char too
	return result;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//удаляет из строки все подстроки
string remove_from_str(string target_, string aim_)
{//target_ - откуда, aim_ - что
	size_t fd_pos;
	while ((fd_pos = target_.find(aim_)) != string::npos)
	{
		target_ = target_.erase(fd_pos, fd_pos + aim_.length() - 1);
	}
	return target_;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//заменяет в строке все подстроки
string replace_in_str(string target_, string what_, string to_what_)
{//target_ - откуда, what_ - что, to_what_ - на что
	size_t fd_pos=0;
	vector<size_t> vec_pos;
	vec_pos.push_back(-1);
	while ((fd_pos = target_.find(what_, fd_pos + 1)) != string::npos)
	{
		vec_pos.push_back(fd_pos);
	}
	for (int i = vec_pos.size() - 1; i > 0; --i)
	{
		target_ = target_.replace(vec_pos[i], what_.length() + 1, to_what_);
	}
	return target_;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void ReplyClient(SHP_MGCP mgcp_, string str_)
{
	BOOST_LOG_SEV(lg, warning) << "Reply is:\n" << str_;
	net_Data->GS(NETDATA::out)->s.send_to(boost::asio::buffer(str_), mgcp_->sender);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void SendModul(int where_, string what_)
{
	BOOST_LOG_SEV(lg, warning) << "SendModul=" << where_ << ":\n" << what_;
	net_Data->GS(NETDATA::in)->s.send_to(boost::asio::buffer(what_), net_Data->GE(where_));
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void LogsInit()
{
	logging::add_file_log
		(
		keywords::auto_flush = true,
		keywords::file_name = init_Params->data[IPar::homePath] + "\\logs\\%Y-%m-%d_MGCP.log",                                        /*< file name pattern >*/
		keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 1), /*< ...or at midnight >*/
		keywords::format = "[%TimeStamp%]:[%ThreadID%] %Message%",                                 /*< log record format >*/
		keywords::open_mode = std::ios_base::app
		);
	logging::core::get()->set_filter(logging::trivial::severity >= stoi(init_Params->data[IPar::logLevel]));
	logging::add_common_attributes();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------