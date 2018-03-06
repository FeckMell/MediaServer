#include "stdafx.h"
#include "Control.h"
using namespace dtmf;


Control::Control()
{
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::Control(...)";
	//vecIOs.resize(1);
	//vecIOstates.resize(1, 0);
	//vecIOs[0].reset(new IO());
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::Control(...) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Preprocessing(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::Preprocessing(...)";
	if (ipl_->data["EventType"] == "cr")
	{
		BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::Preprocessing(...) CR(...)";
		CR(ipl_);
	}
	else if (ipl_->data["EventType"] == "dl")
	{ 
		BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::Preprocessing(...) DL(...)";
		DL(ipl_); 
	}
	else
	{
		BOOST_LOG_SEV(LOG::vecLogs, fatal) << "Control::Preprocessing(...): DEFAULT ERROR IPL was\n" << ipl_->ipl;
		exit(-1);
	}
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::Preprocessing(...) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::CR(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::CR(...) for ID=" << ipl_->data["EventID"];
	//int free_thread = GetFreeThread();
	//BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::CR(...) free_thread=" << free_thread << " Creating point";
	SHP_Point new_point = make_shared<Point>(Point(ipl_));
	vecPoints.push_back(new_point);
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::CR(...) point created and added";
	thread th([new_point,this]()
	{
		new_point->socket->s.async_receive_from(
			boost::asio::buffer(new_point->rawBuf.data, 1000),
			new_point->endPoint,
			boost::bind(&Control::Receive, this, _1, _2, new_point)
			);
		new_point->socket->io->reset();
		new_point->socket->io->run();
	});
	th.detach();
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::CR(...) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DL(SHP_IPL ipl_)
{
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::DL(...) for ID=" << ipl_->data["EventID"];
	SHP_Point found_point = FindPoint(ipl_->data["EventID"]);
	if (found_point == nullptr)
	{
		BOOST_LOG_SEV(LOG::vecLogs, fatal) << "Control::DL(...) found_point == nullptr IPL was\n" << ipl_->ipl;
		exit(-1);
	}
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::DL(...) DeletePoint(found_point)";
	DeletePoint(found_point);
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::DL(...) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Receive(boost::system::error_code ec_, size_t size_, SHP_Point point_)
{
	if (size_ > 12)
	{
		uint8_t bytes[2];

		memcpy(&bytes[0], point_->rawBuf.data + 1, 1);
		memcpy(&bytes[1], point_->rawBuf.data + 12, 1);
		bool result = point_->Analyze(bytes);
		if (result == false)
		{
			BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::Receive(...) ID=" << point_->eventID<< " result == false";
			point_->socket->s.async_receive_from(
				boost::asio::buffer(point_->rawBuf.data, 1000),
				point_->endPoint,
				boost::bind(&Control::Receive, this, _1, _2, point_)
				);
		}
		else
		{
			BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::Receive(...) ID=" << point_->eventID << " result == true";
			DeletePoint(point_);
		}
	}
	else
	{
		BOOST_LOG_SEV(LOG::vecLogs, fatal) << "Control::Receive(...) ERROR size <12";
		exit(-1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DeletePoint(SHP_Point point_)
{
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::DeletePoint(...) ID=" << point_->eventID;
	point_->socket->s.cancel();
	vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
	BOOST_LOG_SEV(LOG::vecLogs, trace) << "Control::DeletePoint(...) END";
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Point Control::FindPoint(string eventid_)
{
	for (auto &point : vecPoints) if (point->eventID == eventid_) return point;
	return nullptr;
}