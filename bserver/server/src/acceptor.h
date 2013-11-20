#ifndef __ACCEPTOR__H
#define __ACCEPTOR__H

#include "pollable.h"
#include "protocol.h"
#include "schedule.h"
#include "connection.h"
#include "event_manager.h"

class Acceptor : public pollable, public ConnectionFactory {
public:
	Acceptor(const char* addr, protocol* proto, schedule* sched, EventManager* emgr);
	virtual ~Acceptor();
public:
	virtual int on_readable();
	virtual int on_writable() { return -1; }
	virtual int on_error() { return -1; }

	virtual const char* name() const { return addr_.c_str(); }
	virtual void destory() { delete this; }
	virtual int idle(const struct timeval& tnow) { return 0; }

	virtual Connection* create(int fd);
	virtual void destory(Connection* conn);
private:
	protocol* proto_;
	schedule* sched_;
	EventManager* emgr_;
	std::string addr_;
	struct sockaddr naddr_;
};

class PassiveConnection : public Connection {
public:
	PassiveConnection(int fd, flow_t flow, protocol* proto, schedule* sched, Acceptor* acceptor)
		: Connection(fd, flow, proto, sched),
		  acceptor_(acceptor)
	{
		assert(acceptor_ != NULL);
	}
public:
	virtual void destory();
private:
	Acceptor* acceptor_;
};

#endif /* __ACCEPTOR__H */


