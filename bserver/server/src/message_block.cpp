#include <algorithm>
#include <cstring>
#include <exception>

#include "message_block.h"
#include "log.h"

MessageBlock::MessageBlock(size_t size)
	: dblock_(new DataBlock(size)), rptr_(0), wptr_(0), cont_(NULL)
{
	// nothing
}

MessageBlock::MessageBlock(byte* addr, size_t size, bool auto_delete)
	: dblock_(new DataBlock(addr, size, auto_delete)), rptr_(0), wptr_(0), cont_(NULL)
{
	// nothing
}

MessageBlock::MessageBlock(DataBlock* dblock)
	: dblock_(dblock), rptr_(0), wptr_(0), cont_(NULL)
{
	assert(dblock_ != NULL);
	dblock_->add_ref();
}
	
MessageBlock::~MessageBlock()
{
	if (cont_ != NULL) {
		delete cont_;
	}

	assert(dblock_ != NULL);
	dblock_->dec_ref();
}

size_t MessageBlock::tsize() const
{
	size_t s = size();

	for (const MessageBlock* c = cont(); c != NULL; c = c->cont()) {
		s += c->size();
	}

	return s;
}

void MessageBlock::reset()
{
	rptr_ = wptr_ = 0;
}

int MessageBlock::resize(size_t nsize)
{
	try {
		DataBlock* ndb = new DataBlock(nsize);
		memcpy(ndb->addr(), rptr(), std::min(nsize, size()));

		wptr_ = size();
		rptr_ = 0;

		dblock_->dec_ref();
		dblock_ = ndb;
	}
	catch (std::exception& ex)
	{
		SYSLOG_ERROR("MessageBlock resize from %ld to %ld failed: %s", (long)size(), (long)nsize, ex.what());
		return -1;
	}
	
	return 0;
}

MessageRequest* MessageRequest::extract(size_t size)
{
	return NULL;
#if 0
	MessageRequest* sub = new (std::nothrow) MessageRequest(this->fd_,
								this->flow_,
								this->dblock(),
								this->peer_addr_,
								this->local_addr_);
	if (sub == NULL) {
		return NULL;
	}

	sub->tr_byte0_ = tr_byte0_;
	sub->tr_last_ = tr_last_;
	sub->r_cnt_ = r_cnt_;
	sub->rptr(this->rptr_);
	sub->wptr(this->rptr_ + size);
	this->rptr(size);

	return sub;
#endif
}
