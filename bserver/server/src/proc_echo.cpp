#include <stdexcept>

#include "proto_h16.h"
#include "message_block.h"
#include "processor.h"
#include "log.h"

class proc_echo : public processor {
public:
	virtual int init();
	virtual void fini();
	virtual int process(int tid, MessageRequest& req, MessageResponse*& rsp);
};

int proc_echo::init()
{
	return 0;
}

void proc_echo::fini()
{
	//
}

int proc_echo::process(int tid, MessageRequest& req, MessageResponse*& rsp)
{
	SYSLOG_DEBUG("echo process %lu bytes data", (unsigned long)req.tsize());
	struct proto_h16_head rh;
	struct proto_h16_res res;
	size_t rsize = req.tsize() + sizeof res - sizeof rh;
	rsp = new MessageResponse(req.flow_, req.fd_, rsize, req.peer_addr_, req.local_addr_);

	res.len = rsize;
	res.cmd = 99;
	res.ver = 1;
	res.syn = 2;
	res.ack = 3;
	res.ret_ = res.len;

	memcpy(rsp->wptr(), &res, sizeof res);
	rsp->wptr(res.len);
	rsp->conn = req.conn;

	return 0;
}

extern "C" processor* create()
{
	try {
		processor* proc = new proc_echo();
		return proc;
	}
	catch (std::exception& ex)
	{
		SYSLOG_ERROR("create failed: %s", ex.what());
	}

	return NULL;
}

extern "C" int destroy(processor* proc)
{
	try {
		proc_echo* eproc = dynamic_cast<proc_echo *>(proc);
		if (eproc != NULL) {
			delete eproc;
			return 0;
		}
	}
	catch (std::exception& ex)
	{
		SYSLOG_ERROR("destroy failed: %s", ex.what());
	}

	return -1;
}

