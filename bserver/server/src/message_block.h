#ifndef __MESSAGE_BLOCK__H
#define __MESSAGE_BLOCK__H

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <new>
#include <stdexcept>
#include <assert.h>

typedef unsigned char byte;
typedef long flow_t;

class DataBlock {
public:
	static byte* allocate(size_t size) { return new (std::nothrow)byte[size]; }
	static void deallocate(byte* p) { delete[] p; }
public:
	DataBlock(size_t size)
		: addr_(NULL), size_(size), refcnt_(1), auto_delete_(true)
	{
		addr_ = allocate(size_);
		if (addr_ == NULL) {
			char buf[512];
			snprintf(buf, sizeof buf, "allocate %lu bytes failed: %m", (unsigned long)size_); 
			throw std::runtime_error(buf);
		}

		return;
	}

	// addr must be allocated from DataBlock::allocate
	// otherwise, destructor may not work well in some platforms, like Windows
	DataBlock(byte *addr, size_t size, bool auto_delete)
		: addr_(addr), size_(size), refcnt_(1), auto_delete_(auto_delete)
	{
		assert(addr != NULL);
	}

	~DataBlock()
	{
		assert(refcnt_ == 0);
		if (auto_delete_ && addr_ != NULL) {
			deallocate(addr_);
		}
	}
private:
	DataBlock(const DataBlock&); // no cc permit
public:
	byte* addr() const { return addr_; }
	size_t size() const { return size_; }
	
	long add_ref() { assert(refcnt_ >= 1); return ++refcnt_; }
	long dec_ref() { long refcnt = --refcnt_; assert(refcnt >= 0); if (!refcnt) delete this; return refcnt; }
private:
	byte* addr_;
	size_t size_;

	long refcnt_;
	bool auto_delete_;
};

class MessageBlock {
public:
	MessageBlock(size_t size);
	MessageBlock(byte* addr, size_t size, bool auto_delete);
	MessageBlock(DataBlock* dblock);
	~MessageBlock();
private:
	MessageBlock(const MessageBlock&); // no cc
public:
	byte* rptr() { return dblock_->addr() + rptr_; }
	byte* wptr() { return dblock_->addr() + wptr_; }
	byte* rptr(ptrdiff_t advance) { return dblock_->addr() + (rptr_ += advance); }
	byte* wptr(ptrdiff_t advance) { return dblock_->addr() + (wptr_ += advance); }
	
	MessageBlock* cont() { return cont_; }
	const MessageBlock* cont() const { return cont_; }
	void cont(MessageBlock* cont) { cont_ = cont; }

	size_t size() const { return wptr_ - rptr_; } // data size between w-r ptr
	size_t space() const { return dblock_->size() - wptr_; } // space after wptr
	size_t bsize() const { return dblock_->size(); } // block size
	size_t tsize() const; // total size, including size of continue blocks'

	void reset();
	int resize(size_t nsize);

	DataBlock *dblock() { return dblock_; }
private:
	DataBlock* dblock_;
	ptrdiff_t rptr_;
	ptrdiff_t wptr_;

	MessageBlock* cont_;
};

class MessageEntry : public MessageBlock {
public:
	MessageEntry(size_t size)
		: MessageBlock(size), next_(NULL)
	{}
	MessageEntry(byte* addr, size_t size, bool auto_delete)
		: MessageBlock(addr, size, auto_delete), next_(NULL)
	{}
	MessageEntry(DataBlock* dblock)
		: MessageBlock(dblock), next_(NULL)
	{}
public:
	MessageEntry* next() { return next_; }
	void next(MessageEntry* next) { next_ = next; }
	MessageEntry** p_next() { return &next_; }
private:
	MessageEntry* next_;
};

class MessageRequest : public MessageEntry {
public:
	MessageRequest(flow_t flow, int fd, size_t isize, struct sockaddr& paddr, struct sockaddr& laddr)
		: MessageEntry(isize), flow_(flow), fd_(fd)
	{
		peer_addr_ = paddr;
		local_addr_ = laddr;
	}
	
	MessageRequest(flow_t flow, int fd, byte* addr, size_t size, bool ad, struct sockaddr& paddr, struct sockaddr& laddr)
		: MessageEntry(addr, size, ad), flow_(flow), fd_(fd)
	{
		peer_addr_ = paddr;
		local_addr_ = laddr;
	}
	
	MessageRequest(flow_t flow, int fd, DataBlock* dblock, struct sockaddr& paddr, struct sockaddr& laddr)
		: MessageEntry(dblock), flow_(flow), fd_(fd)
	{
		peer_addr_ = paddr;
		local_addr_ = laddr;
	}
public:
	MessageRequest* extract(size_t msize);
public:
	flow_t flow_;
	int fd_;

	struct timeval tr_byte0_, tr_last_; // tr_last = enqueue time?
	struct timeval tp_start_, tp_end_;
	int r_cnt_;

	struct sockaddr peer_addr_;
	struct sockaddr local_addr_;

	void *conn;
};

class MessageResponse : public MessageEntry {
public:
	MessageResponse(flow_t flow, int fd, size_t isize, struct sockaddr& paddr, struct sockaddr& laddr)
		: MessageEntry(isize), flow_(flow), fd_(fd)
	{
		peer_addr_ = paddr;
		local_addr_ = laddr;
	}
public:
	struct timeval ts_ready, ts_queue;
	struct timeval ts_byte0, ts_last;
	int s_cnt;

	flow_t flow_;
	int fd_;
	struct sockaddr peer_addr_;
	struct sockaddr local_addr_;

	void *conn;
};

#endif /*!__MESSAGE_BLOCK__H */

