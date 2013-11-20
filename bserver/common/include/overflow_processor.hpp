#ifndef __OVERFLOW_PROCESSOR__
#define __OVERFLOW_PROCESSOR__

#include <time.h>
#include <sys/time.h>
#include <syslog.h>
#include <pthread.h>
#include <Singleton.h>
#include "log.h"

static const long AVG_COST_ALPHA 	= 950;
static const long AVG_COST_BETA 	= 50;
static const long AVG_LOAD_ALPHA	= 950;
static const long AVG_LOAD_BETA 	= 50;
static const long LOAD_VALVE		= 90;

static const long MICRO_SECOND		= 1000000;
static const long LIMIT_MICRO_SECOND	= 800000;
static const long SLEEP_MICRO_SECOND	= 200000;

class processor_parameter
{
public:
	processor_parameter()
	{
		gettimeofday(&start_time_, NULL);
		busy_ = false;
		used_cost_us_ = 0;
	}
	virtual ~processor_parameter()
	{

	}
public:
	void set_start_time()
	{
		gettimeofday(&start_time_, NULL);
		//start_time_ = start_time;
	}
	void set_busy(bool busy)
	{
		busy_ = busy;
	}

	void set_used_cost_us(long used_cost_us)
	{
		used_cost_us_ = used_cost_us;
	}

	void add_used_cost_us(long use_cost)
	{
		used_cost_us_ += use_cost;
	}
	struct timeval get_start_time()
	{
		return start_time_;
	}

	bool is_busy()
	{
		return busy_;
	}

	long get_used_cost_us()
	{
		return used_cost_us_;
	}
	
private:
	struct timeval start_time_;
	volatile long used_cost_us_;
	volatile bool busy_;
};

class overflow_processor
{
public:
	overflow_processor();
	virtual ~overflow_processor();

	friend void* calc_cost_cnt(void* p);
public:
	int start();
	void stop(int cnt);
	//void check();
	void set_limit_cost(long limit_cost_us);
	bool is_over_flow();
	void set_process_cnt(int  process_cnt);
	void set_check_interval(long check_interval_us);
	long get_check_interval();
private:
	bool update(const long ms);

	void Lock();
	void Unlock();

private:
	long limit_cost_us_;
	volatile long avg_cost_us_;
	//volatile long used_cost_us_;
	//struct timeval start_time_;

	//volatile bool busy_;
	volatile bool exit_;
	volatile int process_cnt_;
	volatile long check_interval_us_;
	
	struct processor_parameter** array_parameter_;
	
	pthread_mutex_t lock_;
	pthread_t thread_id_;
};

typedef Singleton<overflow_processor> OVERFLOW_PRO;

void* calc_cost_cnt(void* p)
{
	overflow_processor *overflow = (overflow_processor*)p;

	struct timeval t1,t2;
	gettimeofday(&t1, NULL);

	long check_interval = overflow->get_check_interval();
	while(true)
	{
		//sleep(1);
		usleep(check_interval);
		
		gettimeofday(&t2, NULL);
		long ms = (t2.tv_sec - t1.tv_sec) * 1000 + (t2.tv_usec - t1.tv_usec) / 1000;

		if ( !overflow->update(ms) )
			break;
		t1 = t2;
	}

	return NULL;
}

overflow_processor::overflow_processor()
{
	limit_cost_us_ = 0;
	avg_cost_us_ = 0;
	//used_cost_us_ = 0;
	//busy_ = false;
	exit_ = false;
	process_cnt_ = 0;
	array_parameter_ = NULL;
	check_interval_us_ = 1000000;
	
	pthread_mutex_init(&lock_, NULL);

	int retval = pthread_create(&thread_id_, NULL, calc_cost_cnt, this);
	if ( retval != 0 )
	{
	    	syslog()->error("overflow create thread #%d failed: %s", 1, strerror(retval));
	}
	else
	{
		syslog()->debug("create overflow thread#%d OK: %lu", 1, (unsigned long)thread_id_);
	}
}

overflow_processor::~overflow_processor()
{
	exit_ = true;
	
	if(process_cnt_ > 0)
	{
		if(array_parameter_ != NULL)
		{
			for(int i = 0; i < process_cnt_; i++)
			{
				if(array_parameter_[i] != NULL)
				{
					delete array_parameter_[i];
					array_parameter_[i] = NULL;
				}
			}
			delete[] array_parameter_;
		}
	}
	
	pthread_join(thread_id_, NULL);
	pthread_mutex_destroy(&lock_);
}

void overflow_processor::Lock()
{
	pthread_mutex_lock(&lock_);
}

void overflow_processor::Unlock()
{
	pthread_mutex_unlock(&lock_);
}

bool overflow_processor::update(const long ms)
{
	Lock();

	if ( exit_ )
	{
		Unlock();
		return false;
	}

	long used_cost_us = 0;
	struct timeval end_time;

	gettimeofday(&end_time, NULL);

	for(int i = 0; i < process_cnt_; i++)
	{
		long cost_us = 0;
		processor_parameter* parameter_ = array_parameter_[i];

		if(parameter_ == NULL)
			continue;
		
		if(parameter_->is_busy())
		{
			struct timeval start_time = parameter_->get_start_time();
			cost_us = (end_time.tv_sec - start_time.tv_sec) * MICRO_SECOND + (end_time.tv_usec - start_time.tv_usec);
		}
		else
		{
			cost_us = 0;
		}

		parameter_->set_start_time();
		
		parameter_->add_used_cost_us(cost_us);

		used_cost_us += parameter_->get_used_cost_us();

		parameter_->set_used_cost_us(0);
		
	}

	used_cost_us = used_cost_us/process_cnt_;

	used_cost_us = used_cost_us * 1000 / ms;

	if ( avg_cost_us_ == 0 )
	{
		avg_cost_us_ = used_cost_us;
	}
	else
	{
		avg_cost_us_ = (avg_cost_us_ * AVG_COST_ALPHA + used_cost_us * AVG_COST_BETA)/1000;
	}
	/*
	if(avg_cost_us_ > 0)
		syslog()->error("overflow:update avg_cost_us:%ld, used_cost_us:%ld", avg_cost_us_, used_cost_us);
	*/
	Unlock();

	return true;
}

void overflow_processor::set_limit_cost(long limit_cost_us)
{
	limit_cost_us_ = limit_cost_us;
}

void overflow_processor::set_check_interval(long check_interval_us)
{
	if(check_interval_us_ <= 0)
		return;
	
	check_interval_us_ = check_interval_us;
}

long overflow_processor::get_check_interval()
{
	return check_interval_us_;
}
void overflow_processor::set_process_cnt(int  process_cnt)
{
	if(process_cnt <= 0)
	{
		return;
	}
	else if(process_cnt_ > 0)
	{
		if(array_parameter_ != NULL)
		{
			for(int i = 0; i < process_cnt_; i++)
			{
				if(array_parameter_[i] != NULL)
				{
					delete array_parameter_[i];
					array_parameter_[i] = NULL;
				}
			}
			delete[] array_parameter_;
		}
	}

	process_cnt_ = process_cnt;

	array_parameter_ =  new processor_parameter*[process_cnt_];

	for(int i = 0; i < process_cnt_; i++)
	{
		array_parameter_[i] = new processor_parameter;
	}
}
/*
void overflow_processor::check()
{
	Lock();

	syslog()->error("############### overflow check: avg_cost_us_: %ld", avg_cost_us_);

	if ( avg_cost_us_  > limit_cost_us_ )
	{
		Unlock();
		long sleep_time = MICRO_SECOND - limit_cost_us_;
		struct timeval t1,t2;
		gettimeofday(&t2, NULL);
		usleep(sleep_time);
		gettimeofday(&t1, NULL);
		long ms = (t1.tv_sec - t2.tv_sec)*1000 + (t1.tv_usec - t2.tv_usec)/1000;
		syslog()->error("######### overflow check: usleep time: %ld cost time: %ld", sleep_time, ms);
		return ;
	}

	Unlock();
}
*/
int overflow_processor::start()
{
	int cnt = -1;
	Lock();

	for(int i = 0; i < process_cnt_; i++)
	{
		processor_parameter* parameter_ = array_parameter_[i];
		if(parameter_ != NULL && !parameter_->is_busy())
		{
			parameter_->set_start_time();
			parameter_->set_busy(true);
			cnt = i;
			break;
		}
	}
	Unlock();

	return cnt;
}

void overflow_processor::stop(int cnt)
{
	Lock();

	if(cnt < process_cnt_ && cnt >= 0)
	{
		processor_parameter* parameter_ = array_parameter_[cnt];
		if(parameter_ != NULL)
		{
			struct timeval end_time;
			gettimeofday(&end_time, NULL);
			struct timeval start_time = parameter_->get_start_time();
			long cost_us = (end_time.tv_sec - start_time.tv_sec) * MICRO_SECOND + (end_time.tv_usec - start_time.tv_usec);
			parameter_->set_busy(false);
			parameter_->add_used_cost_us(cost_us);
		}
	}
	Unlock();

}

bool overflow_processor::is_over_flow()
{
	bool bRet = false;

	Lock();

	//syslog()->error("overflow:is_over_flow avg_cost_us:%ld, limit_cost_us:%ld", avg_cost_us_, limit_cost_us_);

	if ( avg_cost_us_  > limit_cost_us_ )
	{
		syslog()->error("overflow:is_over_flow avg_cost_us:%ld, limit_cost_us:%ld", avg_cost_us_, limit_cost_us_);
		bRet = true;
	}
			
	Unlock();

	return bRet;
}
class auto_overflow
{
public:
	auto_overflow()
	{
		cur_process = OVERFLOW_PRO::Instance().start();
		//syslog()->error("overflow start cur_process:%d", cur_process);
	}

	bool is_over_flow()
	{
		return OVERFLOW_PRO::Instance().is_over_flow();
	}
	
	virtual ~auto_overflow()
	{
		OVERFLOW_PRO::Instance().stop(cur_process);
		//syslog()->error("overflow stop cur_process:%d", cur_process);
	}
	
private:
	int cur_process;
	
};

#endif /*__OVERFLOW_PROCESSOR__*/
