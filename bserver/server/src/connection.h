#ifndef __CONNECTION__H
#define __CONNECTION__H

#include <sys/types.h>
#include <sys/socket.h>

#include "beyondy/mutex_cond.hpp"
#include "message_block.h"
#include "message_queue.hpp"
#include "pollable.h"
#include "protocol.h"
#include "schedule.h"

typedef long flow_t;
#define FLOW_NIL -1

class Connection : public pollable {
public:
	Connection(int fd, flow_t flow, protocol* proto, schedule* sched);
	virtual ~Connection();
public:
	virtual int on_readable();
	virtual int on_writable();
	virtual int on_error();

	virtual const char* name() const;
	virtual void destory() = 0;
	virtual int idle(const struct timeval& tnow);
public:
	int response(MessageResponse* rsp);
public:
	int do_read();
	int do_send();

	int init_block();
	int split_block(size_t msize);
	int handle_block();
private:
	flow_t flow_;

	protocol* proto_;
	schedule* sched_;

	struct timeval t_create_;
	struct timeval t_last_readable_;	// timestamp for last received package
	struct timeval t_last_writable_;	// timestamp for last sent package
	
	MessageRequest* mreq_;
	MessageBlock* mbr_;

	beyondy::thread_mutex outlock_;
	MessageQueue<MessageResponse> outq_;

	MessageResponse* mrsp_;
	MessageBlock* mbs_;

	struct sockaddr laddr_;			// local address
	struct sockaddr paddr_;			// peer address
	char name_[256];
};

class ConnectionFactory {
public:
	virtual ~ConnectionFactory() { /* nothing */ }
public:
	virtual Connection* create(int fd) = 0;
	virtual void destory(Connection* conn) = 0;
};

#endif /*!__CONNECTION__H */
