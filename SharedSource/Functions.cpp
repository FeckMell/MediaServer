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
string copy_single_n_line(string target_, int n_)
{
	size_t fd_pos_prev = 0;
	size_t fd_pos_next = 0;
	for (int i = -1; i < n_; ++i)
	{
		size_t fd_pos_cur = target_.find("\n", fd_pos_next + 1);
		if (fd_pos_cur != string::npos)
		{
			fd_pos_prev = fd_pos_next + 1;
			fd_pos_next = fd_pos_cur;
		}
		else if (i + 2 != n_) return "";
	}
	if (fd_pos_prev != 1) return target_.substr(fd_pos_prev, fd_pos_next - fd_pos_prev);
	else return target_.substr(0, fd_pos_next);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string remove_single_line(string target_, string aim_)
{
	size_t fd_pos = target_.find(aim_);
	if (fd_pos == string::npos){ return target_; }

	return target_.erase(fd_pos, aim_.length());
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string remove_single_line(string target_, int n_)
{
	size_t fd_pos_prev = 0;
	size_t fd_pos_next = 0;
	for (int i = -1; i < n_; ++i)
	{
		size_t fd_pos_cur = target_.find("\n", fd_pos_next + 1);
		if (fd_pos_cur != string::npos)
		{
			fd_pos_prev = fd_pos_next + 1;
			fd_pos_next = fd_pos_cur;
		}
		else if (i + 2 != n_) return target_;
	}
	if (fd_pos_prev != 1) return target_.erase(fd_pos_prev, fd_pos_next - fd_pos_prev + 1);
	else return target_.erase(0, fd_pos_next + 1);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
string replace_line(string target_, string was_, string now_)
{
	size_t fd_pos = target_.find(was_);
	return target_.replace(fd_pos, was_.length(), now_);
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int find_line(string target_, string what_)
{
	string temp = "a";
	int line = 0;
	while (temp != "")
	{
		temp = copy_single_n_line(target_, line);
		if (temp.find(what_) != string::npos)
			return line;
		line++;
	}
	return -1;
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