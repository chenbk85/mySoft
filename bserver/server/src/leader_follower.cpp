#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "leader_follower.h"
#include "beyondy/timedout_countdown.hpp"

LeaderFollowerManager::LeaderFollowerManager(int tcnt, void* (*entry)(void *), void *data)
	: leader_(999999),
	  mutex_cond_(NULL, NULL),
	  workders_(tcnt)
{
	for (int i = 0; i < tcnt; ++i) {
		pthread_t tid;	
		retval = pthread_create(&tid, NULL, entry, data);
		if (retval != 0) {
			errno = retval;
			char buf[512];
			snprintf(buf, sizeof buf, "create %dth worker of %d for leader-follower failed: %m", i, tcnt);
			throw std::runtime_error(buf);
		}

		workers_[i].id_ = i;
		workers_[i].tid_ = tid;
	}
}

void LeaderFollowerManager::start()
{
	mutex_cond_.lock();
	leader_ = -1;
	mutex_cond_.broadcast();
	mutex_cond_.unlock();
}
	
int LeaderFollowerManager::wait_leadership(int id, long ms)
{
	beyondy::TimedoutCountdown tc(ms);
	long left;
	int retval;

	mutex_cond_.lock();
	do {
		left = tc.Update();
		// calc left time
		retval = mutex_cond_.timedwait(left);
		if (retval < 0) {
			if (errno == ETIMEDOUT || errno != EINTR) {
				mutex_cond_.unlock();
				return -1; // with errno
			}

			// ignore EINTR
			continue;
		}

		if (leader_ < 0) {
			// got it
			leader_ = workers_[id].id_;
			mutex_cond_.unlock();
			return 0;
		}

		// otherwise, someone get leader first
		// wait again unless timed out
	} while (left != 0);

	errno = ETIMEDOUT;
	return -1;
}

void LeaderFollowerManager::release_leadership(int id)
{
	mutex_cond_.lock();
	assert(id == leader_);
	leader_ = -1;
	mutex_cond_.signal();
	mutex_cond_.unlock();
}

