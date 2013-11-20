#ifndef __EVENT_MANAGER__H
#define __EVENT_MANAGER__H

#include <sys/epoll.h>
#include "connection.h"


class EventManager {
public:
	EventManager(int tcnt, size_t esize = 1024);
	~EventManager();
public:
	int add_poller(pollable* conn, uint32_t event);
	int mod_poller(pollable* conn, uint32_t event);
	int del_poller(pollable* conn);

	int add_timer();
	int mod_timer();
	int del_timer();
	
	int loop(long ms);

	flow_t next_flow();
private:
	void handle_events(struct epoll_event *events, int count);
	void handle_event(struct epoll_event *event);
	void close_pollable(pollable* p);

	void *thread_entry(void *p);
private:
	int epoll_fd_;
	flow_t next_flow_;
	std::vector<pthread_t> threads;
	size_t epoll_size_;
	size_t epoll_next_;
	struct epoll_event *epoll_events_;
	struct list_head phead_;


	beyondy::thread_cond_t tcond_;
};



#endif /* __EVENT_MANAGER__H */


