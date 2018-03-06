#include "stdafx.h"
#include "Control.h"

Control::Control()
{
	BOOST_LOG_SEV(lg, trace) << "Control::Control(...)";
	vecIOs.resize(1);
	vecIOstates.resize(1, 0);
	vecIOs[0].reset(new IO());
	BOOST_LOG_SEV(lg, trace) << "Control::Control(...) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Preprocessing(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(lg, trace) << "Control::Preprocessing(...)";
	if (ipl_->data["EventType"] == "cr")
	{
		BOOST_LOG_SEV(lg, trace) << "Control::Preprocessing(...) CR(...)";
		CR(ipl_);
	}
	else if (ipl_->data["EventType"] == "dl")
	{ 
		BOOST_LOG_SEV(lg, trace) << "Control::Preprocessing(...) DL(...)";
		DL(ipl_); 
	}
	else
	{
		BOOST_LOG_SEV(lg, fatal) << "Control::Preprocessing(...): DEFAULT ERROR IPL was\n" << ipl_->ipl;
		exit(-1);
	}
	BOOST_LOG_SEV(lg, trace) << "Control::Preprocessing(...) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::CR(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(lg, trace) << "Control::CR(...) for ID=" << ipl_->data["EventID"];
	int free_thread = GetFreeThread();
	BOOST_LOG_SEV(lg, trace) << "Control::CR(...) free_thread=" << free_thread << " Creating point";
	SHP_Point new_point = make_shared<Point>(Point(ipl_, *vecIOs[free_thread].get()));
	vecPoints.push_back(new_point);
	BOOST_LOG_SEV(lg, trace) << "Control::CR(...) point created and added";
	new_point->socket->s.async_receive_from(
		boost::asio::buffer(rawBuf.data, 1000),
		new_point->endPoint,
		boost::bind(&Control::Receive, this, _1, _2, new_point)
		);
	if (vecIOstates[free_thread] == 0)
	{
		BOOST_LOG_SEV(lg, trace) << "Control::CR(...) vecIOstates[free_thread] == 0";
		thread th(&Control::RunIO, this, free_thread);
		th.detach();
	}
	else
	{
		BOOST_LOG_SEV(lg, trace) << "Control::CR(...) vecIOstates[free_thread] != 0";
		vecIOstates[free_thread]++;
	}
	BOOST_LOG_SEV(lg, trace) << "Control::CR(...) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DL(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(lg, trace) << "Control::DL(...) for ID=" << ipl_->data["EventID"];
	SHP_Point found_point = FindPoint(ipl_->data["EventID"]);
	if (found_point == nullptr)
	{
		BOOST_LOG_SEV(lg, fatal) << "Control::DL(...) found_point == nullptr IPL was\n" << ipl_->ipl;
		exit(-1);
	}
	BOOST_LOG_SEV(lg, trace) << "Control::DL(...) DeletePoint(found_point)";
	DeletePoint(found_point);
	BOOST_LOG_SEV(lg, trace) << "Control::DL(...) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Receive(boost::system::error_code ec_, size_t size_, SHP_Point point_)
{
	if (size_ > 12)
	{
		uint8_t bytes[2];

		memcpy(&bytes[0], rawBuf.data + 1, 1);
		memcpy(&bytes[1], rawBuf.data + 2, 1);
		bool result = point_->Analyze(bytes);
		if (result == false)
		{
			BOOST_LOG_SEV(lg, trace) << "Control::Receive(...) ID=" << point_->eventID<< " result == false";
			point_->socket->s.async_receive_from(
				boost::asio::buffer(rawBuf.data, 1000),
				point_->endPoint,
				boost::bind(&Control::Receive, this, _1, _2, point_)
				);
		}
		else
		{
			BOOST_LOG_SEV(lg, trace) << "Control::Receive(...) ID=" << point_->eventID << " result == true";
			DeletePoint(point_);
		}
	}
	else
	{
		BOOST_LOG_SEV(lg, fatal) << "Control::Receive(...) ERROR size <12";
		exit(-1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DeletePoint(SHP_Point point_)
{
	BOOST_LOG_SEV(lg, trace) << "Control::DeletePoint(...) ID=" << point_->eventID;
	point_->socket->s.cancel();
	vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
	BOOST_LOG_SEV(lg, trace) << "Control::DeletePoint(...) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::RunIO(int i_)
{
	BOOST_LOG_SEV(lg, trace) << "Control::RunIO(...) Control::RunIO(...) for i="<<i_;
	vecIOs[i_]->reset();
	vecIOstates[i_] = 1;
	vecIOs[i_]->run();
	vecIOstates[i_] = 0;
	BOOST_LOG_SEV(lg, trace) << "Control::RunIO(...) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Point Control::FindPoint(string eventid_)
{
	for (auto &point : vecPoints)
		if (point->eventID == eventid_) return point;
	return nullptr;
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
int Control::GetFreeThread()
{
	for (int i = 0; i < (int)vecIOstates.size(); ++i)
		if (vecIOstates[i] < 10) return i;

	int new_size = (int)vecIOstates.size() + 1;
	vecIOs.resize(new_size);
	vecIOs[new_size - 1].reset(new IO());
	vecIOstates.resize(new_size, 0);
	return (int)vecIOstates.size();

}