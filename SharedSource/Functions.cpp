#include "Functions.h"

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
void LogsInit(string my_modul_name_)
{
	string log_path = init_Params->data[STARTUP::homePath] + "\\logs\\%Y-%m-%d_" + my_modul_name_ + ".log";
	boost::log::add_file_log
		(
		boost::log::keywords::auto_flush = true,
		boost::log::keywords::file_name = log_path,                                        /*< file name pattern >*/
		boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 1), /*< ...or at midnight >*/
		boost::log::keywords::format = "[%TimeStamp%]:[%ThreadID%] %Message%",                                 /*< log record format >*/
		boost::log::keywords::open_mode = std::ios_base::app
		);
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= stoi(init_Params->data[STARTUP::logLevel]));
	boost::log::add_common_attributes();
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void init_ffmpeg()
{
	BOOST_LOG_SEV(lg, trace) << "init_ffmpeg()";
	av_log_set_level(0);
	av_register_all();
	avcodec_register_all();
	avfilter_register_all();
	avformat_network_init();
	BOOST_LOG_SEV(lg, trace) << "init_ffmpeg(): DONE";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------