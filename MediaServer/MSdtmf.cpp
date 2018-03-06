#include "stdafx.h"
#include "MSdtmf.h"
using namespace dtmf;


Control::Control()
{	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::Preprocessing(string message_)
{
	SHP_IPL ipl = make_shared<IPL>(IPL(message_));

	if (ipl->data["EventType"] == "cr") { CR(ipl); }
	else if (ipl->data["EventType"] == "dl") {DL(ipl);}
	else
	{

		exit(-1);
	}
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::CR(SHP_IPL ipl_)
{
	SHP_Point new_point = make_shared<Point>(Point(ipl_));
	vecPoints.push_back(new_point);
	
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
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DL(SHP_IPL ipl_)
{
	
	SHP_Point found_point = FindPoint(ipl_->data["EventID"]);
	if (found_point == nullptr)
	{
		
		exit(-1);
	}
	
	DeletePoint(found_point);
	
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
			
			point_->socket->s.async_receive_from(
				boost::asio::buffer(point_->rawBuf.data, 1000),
				point_->endPoint,
				boost::bind(&Control::Receive, this, _1, _2, point_)
				);
		}
		else
		{
			
			DeletePoint(point_);
		}
	}
	else
	{
		
		exit(-1);
	}
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
void Control::DeletePoint(SHP_Point point_)
{
	
	point_->socket->s.cancel();
	vecPoints.erase(std::remove(vecPoints.begin(), vecPoints.end(), point_), vecPoints.end());
	
}
//*///------------------------------------------------------------------------------------------
//*///------------------------------------------------------------------------------------------
SHP_Point Control::FindPoint(string eventid_)
{
	for (auto &point : vecPoints) if (point->eventID == eventid_) return point;
	return nullptr;
}