/* message_queue.hpp
 *
**/
#ifndef __MESSAGE_QUEUE__HPP
#define __MESSAGE_QUEUE__HPP

#include "beyondy/mutex_cond.hpp"
#include "message_block.h"

template <typename _MB>	// MessageRequest, or MessageResponse
class MessageQueue {
public:
	MessageQueue(size_t qlimit = -1U);
	~MessageQueue();
private:
	MessageQueue(const MessageQueue&); // no cc
public:
	_MB* get(long ms = -1);
	int put(_MB *mb, long ms = -1);
	bool empty() const { return mlast_ == &mhead_; }
private:
	size_t qsize_;
	size_t qlimit_;
	beyondy::thread_mutex mutex_;
	beyondy::thread_cond slot_cond_;
	beyondy::thread_cond item_cond_;

	_MB *mhead_;
	_MB **mlast_;
};

template <typename _MB>
MessageQueue<_MB>::MessageQueue(size_t qlimit /* = -1U */)
	: qsize_(0), qlimit_(qlimit), 
	  mutex_(NULL), slot_cond_(mutex_, NULL), item_cond_(mutex_, NULL),
	  mhead_(NULL), mlast_(&mhead_)
{
	// nothing
}

template <typename _MB>
MessageQueue<_MB>::~MessageQueue()
{
	_MB* mb = mhead_;
	while (mb != NULL) {
		_MB* next = (_MB*)mb->next();
		delete mb;
		mb = next;
	}
}

template <typename _MB>
_MB* MessageQueue<_MB>::get(long ms /* = -1 */)
{
	beyondy::auto_lock<typeof mutex_> al(mutex_);

	while (mhead_ == NULL) {
		if (item_cond_.timedwait(ms) < 0) {
			return NULL;
		}
	}
	
	_MB* mb = dynamic_cast<_MB *>(mhead_);
	if ((mhead_ = (_MB *)mb->next()) == NULL) {
		mlast_ = &mhead_;
	}

	--qsize_;
	slot_cond_.signal();
	
	return mb;
}

template <typename _MB>
int MessageQueue<_MB>::put(_MB *mb, long ms /* = -1 */)
{
	beyondy::auto_lock<typeof mutex_> al(mutex_);

	if (qsize_ >= qlimit_) {
		if (slot_cond_.timedwait(ms) < 0) {
			errno = EOVERFLOW;
			return -1;
		}
	}
	
	*mlast_ = mb;
	mlast_ = (_MB **)mb->p_next();

	++qsize_;
	item_cond_.signal();

	return 0;
}

#endif /*! __MESSAGE_QUEUE__HPP */
