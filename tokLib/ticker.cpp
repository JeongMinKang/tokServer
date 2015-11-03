#include "preCompiled.h"
#include "ticker.h"

namespace tokLib 
{

ticker::ticker() : 
    timeDuration_(0), tickCount_(0), stop_(false)
{
}

ticker::~ticker()
{
	if (!stop_)
		stop();
}

void ticker::start(int milliSec, function<void (int64_t)> functor)
{
	if (thread_) 
		return;
    
    timeDuration_ = chrono::milliseconds(milliSec);
    callback_ = functor;

    thread_.reset(new thread(bind(&ticker::tick, this)));
}

void ticker::stop()
{
	stop_=true;

	cond_.notify_one();
	if (thread_) thread_->join();
}

void ticker::tick()
{
	while (!stop_) 
    {
        unique_lock<mutex> lock(mutex_);
        if (cond_.wait_for(lock, timeDuration_) == cv_status::timeout) 
        {
			// timeout
			++tickCount_;
            if (callback_)
				callback_(tickCount_);
        }
	}
}

} // namespace end
