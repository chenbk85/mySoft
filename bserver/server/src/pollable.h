#ifndef __POLLABLE__H
#define __POLLABLE__H

#include <sys/time.h>
#include <beyondy/list.h>

class pollable;
class poller_creator;

class poller_creator {
public:
	virtual pollable* create();
	virtual void destory(pollable* poller);
	virtual ~poller_creator() {}
};

class pollable {
public:
	pollable() : fd_(-1) {}
	pollable(int fd) : fd_(fd) {}
	virtual ~pollable() {}
public:
	int fd() const { return fd_; }
	void fd(int _fd) { fd_ = _fd; }

	struct list_head* list_ptr() { return &list_entry_; }
	static pollable* from_list_ptr(struct list_head *pl) { return list_entry(pl, pollable, list_entry_); }
public:
	virtual int on_readable() = 0;
	virtual int on_writable() = 0;
	virtual int on_error() = 0;

	virtual const char* name() const = 0;
	virtual void destory() = 0;
	virtual int idle(const struct timeval& tnow) = 0;
private:
	int fd_;
	struct list_head list_entry_;
};

#endif /*!__POLLABLE__H */

