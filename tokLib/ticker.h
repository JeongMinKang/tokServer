#pragma once

namespace tokLib 
{

class ticker : private boost::noncopyable
{    
private:
    chrono::milliseconds timeDuration_;

	int64_t tickCount_;

    bool stop_;
    mutex mutex_;
    shared_ptr<thread> thread_;
    condition_variable cond_;
    function<void (int64_t)> callback_;

	void tick();

public:
    ticker();
	~ticker();
    void start(int milliSec, function<void (int64_t)> functor);
	void start();
	void stop();
};

} // namespace end

