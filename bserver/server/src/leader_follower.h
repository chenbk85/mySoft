/* leader_follower.h
 *
**/
#ifndef __LEADER_FOLLOWER__H
#define __LEADER_FOLLOWER__H

#include <sys/time.h>
#include <vector>
#include <pthread.h>

#include "beyondy/mutex_cond.hpp"

struct worker_info {
	int id_;
	pthread_t tid_;

	struct timeval leader_start_;
	struct timeval leader_stop_;
	long leader_time_; //us
};

class LeaderFollowerManager {
public:
	LeaderFollowerManager(int tcnt);
public:
	void start();
	int wait_leadership(long ms);
	void release_leadership();
private:
	volatile int leader_;
	beyondy::thread_mutex_cond mutex_cond_;
	std::vector<worker_info> workers_;
};

#endif /*! __LEADER_FOLLOWER__H */

