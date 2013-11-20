#ifndef __SCHED_MT__H
#define __SCHED_MT__H

#include "processor.h"
#include "message_block.h"
#include "message_queue.hpp"
#include "schedule.h"

class schedule_mt : public schedule {
public:
	schedule_mt(int tcnt, processor* proc);
	virtual ~schedule_mt();
public:
	static void* thread_entry(void *p);
	void* thread_entry(int tid);
public:
	int on_request(MessageRequest* req);
	int response(MessageResponse* rsp);
public:
	static void _register();
	static void unregister();
private:
	struct thread_state {
		schedule_mt* sched;
		pthread_t tid;
		int tno;
	};

	int tcnt_;
	struct thread_state *tdata_;

	MessageQueue<MessageRequest> inq_;
	MessageQueue<MessageResponse> outq_;
};

#endif /*! __SCHED_MT__H */
