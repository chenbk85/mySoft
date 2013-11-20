/* timedout_countdown.hpp
 * Copyright by beyondy, 2008-2010
 * All rights reserved.
 * 
 * 1, beyondy, jan 23, 2008
 *    a simple timer countdown implementation.
 * 2, beyondy, feb 7, 2008
 *    add Restart method.
 * 3, beyondy, feb 8, 2008
 *    Update call During instead of calculating itself during.
**/
#ifndef __BEYONDY__TIMEDOUT_COUNTDOWN__HPP
#define __BEYONDY__TIMEDOUT_COUNTDOWN__HPP

#define TV_DMS(t1, t2) (((t2)->tv_sec  - (t1)->tv_sec ) * 1000 + \
			((t2)->tv_usec - (t1)->tv_usec) / 1000)
	

#if defined WIN32

static int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	long tick = GetTickCount();
	tv->tv_sec = tick / 1000;
	tv->tv_usec = (tick % 1000) * 1000;
	return 0;
}

#else /* defined WIN32 */

#include <sys/time.h>
#include <stdio.h>

#endif /*!defined WIN32 */

namespace beyondy {
class TimedoutCountdown {
public:
	TimedoutCountdown(long timedout = -1) : timedout_(timedout) {
		gettimeofday(&start_tv_, NULL);
	}

	long Timedout() const { return timedout_; }
	void Timedout(long timedout) { timedout_ = timedout; }

	void Restart() {
		gettimeofday(&start_tv_, NULL);
	}
	
	long Elapsed() const {
		struct timeval tn;
		gettimeofday(&tn, NULL);
		return (tn.tv_sec  - start_tv_.tv_sec ) * 1000 + \
		       (tn.tv_usec - start_tv_.tv_usec) / 1000;
	}
	
	long Update(long *elapsed = NULL) const {
		if (timedout_ < 0) return -1;		// never timeout

		long past = Elapsed();
		if (elapsed != NULL) *elapsed = past;
		past = timedout_  - past;

		return (past < 1) ? 0 : past;		// return left time
	}

private:
	long timedout_; /* 0: check only, > 0: check in time(ms), -1: wait for ever */
	struct timeval start_tv_;
}; /* class TimedoutCountdown */
}; /* namespace beyondy */

#endif /*! __BEYONDY__TIMEDOUT_COUNTDOWN__HPP */

