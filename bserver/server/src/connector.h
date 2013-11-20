/* connector.h
 *
**/
#ifndef __CONNECTOR__H
#define __CONNECTOR__H

#include "pollable.h"
#include "protocol.h"
#include "schedule.h"
#include "event_manager.h"
#include "connection.h"
#include "message_block.h"
#include "message_queue.hpp"

class Connector : public pollable, public ConnectionFactory {
public:
	Connector(const char* addr, protocol* proto, schedule* sched, EventManager* emgr);
	virtual ~Connector();
public:
	virtual int on_readable();
	virtual int on_writable();
	virtual int on_error();

	virtual Connection* create(int fd);
	virtual void destory(Connection* conn);
public:
	int send(MessageRequest* mr);
	MessageQueue<MessageResponse>& PendingQueue() { return pending_q_; }
private:
	int concurr_count_;
	int active_count_;
	MessageQueue<MessageResponse> pending_q_;
	protocol* proto_;
	schedule* sched_;
	EventManager* emgr_;
	// TODO: route manager
};

class ActiveConnection : public Connection {
public:
	ActiveConnection(int fd, flow_t flow, protocol* proto, schedule* sched, Connector* connector);
public:
	virtual int on_writable();
	virtual void destory();
private:
	Connector* connector_;
};

#endif /*!__CONNECTOR__H */
