#pragma once
#include <condition_variable>
template<typename _T>
class ThreadedSet
{
public:
	_T find(const _T& key)
	{
		return *cll_.find(key);
	}
	_T regnew(const _T& keyBase, const _T& inc)
	{
		lock lk(mutex_);
		_T result = keyBase;
		while (cll_.find(result) != cll_.cend())
			result += inc;

		cll_.insert(result);
		return result;
	}

	void reg(const _T& key)
	{
		lock lk(mutex_);
		cll_.insert(key);
	}

	void unreg(const _T& key)
	{
		lock lk(mutex_);
		cll_.erase(key);
	}
	void clear()
	{
		lock lk(mutex_);
		cll_.clear();
	}
	bool empty()const
	{
		lock lk(mutex_);
		return cll_.empty();
	}
	void for_each(std::function<void(const _T&)> fn)
	{
		lock lk(mutex_);
		for (const _T& entry : cll_)
		{
			fn(entry);
		}
	}
	size_t size()const{
		lock lk(mutex_);
		return cll_.size();
	}


private:
	std::set<_T>		cll_;
	mutable std::mutex	mutex_;
};

template <typename _T>
class CThreadedCircular : boost::noncopyable
{
public:
	typedef std::unique_lock<std::mutex> ulocker;
	CThreadedCircular(size_t sz, bool bBlockIN=true) 
		: buffer_(sz), bBlockIN_(bBlockIN){
		bTerminated_ = false;
	}
	//-----------------------------------------------------------------------
	void push(const _T& val)
	{
		//cout << "\n     Call from push 1 rand:";
		ulocker lk(mutex_);
		//cout << "\n     Call from push 2 rand:";
		if (bBlockIN_)
		{
			condNotFull_.wait(lk,
				[this]{return !buffer_.full() || bTerminated_; });
		}
		//cout << "\n     Call from push 3 rand:";
		if (!bTerminated_)
		{
			assert(!bBlockIN_ || !buffer_.full());
			buffer_.push_back(val);
			condNotEmpty_.notify_one();
		}
		//cout << "\n     exit from push 4 rand:";
	}
	//-----------------------------------------------------------------------
	bool try_pop(_T& retVal)
	{
		lock lk(mutex_);
		const bool bOK = !buffer_.empty();
		if (bOK)
		{
			retVal = buffer_.front();
			buffer_.pop_front();
			condNotFull_.notify_one();
		}
		return bOK;
	}

	//-----------------------------------------------------------------------
	_T pop()
	{
		_T retVal;
		//cout << "\n        Call from pop() 1 rand:";
		ulocker lk(mutex_);
		//cout << "\n        Call from pop() 2 rand:";
		condNotEmpty_.wait(lk,[this]{return (!buffer_.empty() || bTerminated_); } );// тут
		//cout << "\n        Call from pop() 3 rand:";

		if (!bTerminated_)
		{
			assert(!buffer_.empty());

			retVal = buffer_.front();
			buffer_.pop_front();
			condNotFull_.notify_one();
		}
		//cout << "\n        exit from pop() rand:";
		return retVal;
	}

	//-----------------------------------------------------------------------
	void resume()
	{
		bTerminated_ = false;
	}

	//-----------------------------------------------------------------------
	void terminate()
	{
		bTerminated_ = true;
		lock lk(mutex_);
		buffer_.clear();	
		condNotFull_.notify_one();
		condNotEmpty_.notify_one();
	}
	bool full()const{	lock lk(mutex_); return buffer_.full(); }
	bool empty()const{	lock lk(mutex_); return buffer_.empty(); }
	size_t size()const{	lock lk(mutex_); return buffer_.size(); }
	bool isTerminated()const{ return bTerminated_ == true; }
private:
	mutable std::mutex			mutex_;
	boost::circular_buffer<_T>	buffer_;
	std::condition_variable		condNotEmpty_;
	std::condition_variable		condNotFull_;
	std::atomic<bool>			bcondNotEmpty_ = true;//1
	std::atomic<bool>			bcondNotFull_=true;//1
	std::atomic<bool>			bTerminated_=true;
	const bool	bBlockIN_;

};

