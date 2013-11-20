/* sched_mt.cpp
 *
**/
#include "processor.h"
#include "connection.h"
#include "sched_mt.h"
#include "log.h"

schedule_mt::schedule_mt(int tcnt, processor* proc)
	: schedule(proc), tcnt_(tcnt), tdata_(new thread_state[tcnt]), inq_(-1), outq_(-1)
{
	for (int i = 0; i < tcnt_; ++i) {
		tdata_[i].tno = i;
		tdata_[i].sched = this;
		int retval = pthread_create(&tdata_[i].tid, NULL, thread_entry, &tdata_[i]);
		if (retval != 0) {
			errno = retval;
			char buf[512];
			snprintf(buf, sizeof buf, "create %dth thread failed: %m", i);
			SYSLOG_ERROR("%s", buf);
			throw std::runtime_error(buf);
		}
	}
}

schedule_mt::~schedule_mt()
{
	// TODO:
}

int schedule_mt::on_request(MessageRequest* req)
{
#if 0
	size_t rsize = req->tsize() + 4;
	MessageResponse* rsp = new MessageResponse(req->flow_, req->fd_, rsize, req->peer_addr_, req->local_addr_);
	rsp->wptr(rsize);
        rsp->conn = req->conn;
	if (response(rsp) < 0) delete rsp;
	delete req;
	return 0;
#endif
	int retval = inq_.put(req, 10);
	return retval;
}

int schedule_mt::response(MessageResponse* rsp)
{
	Connection* conn = (Connection *)rsp->conn; // TODO: get message's connection. how?
	int retval;

	if (conn == NULL /* || !valid() */) {
		SYSLOG_ERROR("response %lu data failed: no connection found by flow-%lu",
				(unsigned long)rsp->tsize(), rsp->flow_);
		return -1;
	}

	retval = conn->response(rsp);
	if (retval < 0) {
		SYSLOG_ERROR("response %lu data to %s failed: %m",
				(unsigned long)rsp->tsize(), conn->name());
		return retval;
	}

	return 0;
}

void* schedule_mt::thread_entry(void *p)
{
	struct thread_state *ts = (struct thread_state *)p;
	return ts->sched->thread_entry(ts->tno);
}

void* schedule_mt::thread_entry(int tid)
{
	while (!is_stopped()) {
		MessageRequest* req = inq_.get(100);
		if (req == NULL) {
			SYSLOG_DEBUG("no request got");
			continue;
		}

		MessageResponse* rsp = NULL;
		int retval = proc()->process(tid, *req, rsp);
		
		if (retval < 0) {
			assert(rsp != NULL);
			SYSLOG_ERROR("prcess failed: %m");
			// ignore it
		}
		else if (rsp != NULL) {
			retval = response(rsp);
			if (retval < 0) {
				delete rsp;
				SYSLOG_ERROR("response failed: %d", errno);
			}
		}
		else {
			// no response here
			// skip
		}

		delete req;
	}
	
	return NULL;
}

class MtScheduleFactory : public ScheduleFactory {
public:
	schedule* create(processor* proc, const std::map<std::string, std::string>& params)
	{
		int tcnt = 10;
		schedule* sched = new schedule_mt(tcnt, proc);
		return sched;
	}
	
	void destroy(schedule* sched)
	{
		try {
			schedule_mt* s = dynamic_cast<schedule_mt *>(sched);
			delete s;
		}
		catch (std::exception& ex) {
			// TODO:
		}
	}
};

static const char* fn = "multi-thread";
static MtScheduleFactory sf;
//static void _register(void) __attribute__((constructor));
//static void unregister(void) __attribute__((destructor));

void schedule_mt::_register(void)
{
	ScheduleFactory::_register(fn, &sf);
}

void schedule_mt::unregister(void)
{
	ScheduleFactory::unregister(fn);
}

