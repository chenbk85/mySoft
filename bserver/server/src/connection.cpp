#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "beyondy/to_string.hpp"
#include "beyondy/xbs_io.h"
#include "connection.h"
#include "log.h"

Connection::Connection(int _fd, flow_t flow, protocol* proto, schedule* sched)
	: pollable(_fd), flow_(flow), proto_(proto), sched_(sched), 
	  mreq_(NULL), mbr_(NULL), outlock_(NULL), outq_(-1), 
	  mrsp_(NULL), mbs_(NULL)
{
	socklen_t slen = sizeof(laddr_);
	if (getsockname(fd(), &laddr_, &slen) < 0) {
		char buf[512];
		snprintf(buf, sizeof buf, "getsockname for conn-%d-#%lu failed: %m",
				fd(), (unsigned long)flow);

		SYSLOG_ERROR("%s", buf);
		throw std::runtime_error(buf);
	}
	
	slen = sizeof(paddr_);
	if (getpeername(fd(), &paddr_, &slen) < 0) {
		char buf[512];
		snprintf(buf, sizeof buf, "getpeername for conn-%d-#%lu failed: %m",
				fd(), (unsigned long)flow);

		SYSLOG_ERROR("%s", buf);
		throw std::runtime_error(buf);
	}

	if (laddr_.sa_family == PF_INET) {
		char lb[INET_ADDRSTRLEN];
		char pb[INET_ADDRSTRLEN];

		struct sockaddr_in *lin = (struct sockaddr_in *)&laddr_;
		struct sockaddr_in *pin = (struct sockaddr_in *)&paddr_;

		inet_ntop(lin->sin_family, (void *)&lin->sin_addr, lb, sizeof(lb));
		inet_ntop(pin->sin_family, (void *)&pin->sin_addr, pb, sizeof(pb));

		snprintf(name_, sizeof name_, "conn-%d-#%lu(%s:%d <- %s:%d)",
				fd(), (unsigned long)flow_,
				lb, ntohs(lin->sin_port),
				pb, ntohs(pin->sin_port));
	}
	else if (laddr_.sa_family == PF_INET6) {
		char lb[INET6_ADDRSTRLEN];
		char pb[INET6_ADDRSTRLEN];
		
		struct sockaddr_in6 *lin = (struct sockaddr_in6 *)&laddr_;
		struct sockaddr_in6 *pin = (struct sockaddr_in6 *)&paddr_;

		inet_ntop(lin->sin6_family, (void *)&lin->sin6_addr, lb, sizeof(lb));
		inet_ntop(pin->sin6_family, (void *)&pin->sin6_addr, pb, sizeof(pb));
		
		snprintf(name_, sizeof name_, "conn-%d-#%lu(%s:%d <- %s:%d)",
				fd(), (unsigned long)flow_,
				lb, ntohs(lin->sin6_port),
				pb, ntohs(pin->sin6_port));
	}
	else {
		snprintf(name_, sizeof name_, "conn-%d-#%lu(ukn-family)", fd(), (unsigned long)flow_);
	}

	return;
}

Connection::~Connection()
{
	// TODO:
}

// for new package coming
int Connection::init_block()
{
	try {
		// TODO: isize is from config
		// TODO: allocate from pool
		mreq_ = new MessageRequest(fd(), flow_, 128, paddr_, laddr_);
		mreq_->conn = (void *)(Connection *)this;
	}
	catch (std::exception& ex) {
		SYSLOG_ERROR("allocate request for %s failed by except: %s", name(), ex.what());
		return -1;
	}
	
	if (mreq_ == NULL) {
		SYSLOG_ERROR("allocate request for %s failed: %m", name());
		return -1;
	}

	mbr_ = mreq_;
	gettimeofday(&mreq_->tr_byte0_, NULL);
	mreq_->r_cnt_ = 0;

	return 0;
}

int Connection::split_block(size_t msize)
{
	//MessageRequest* sub;
	int retval = -1;
#if 1
	assert(0);
#else
	// TODO: need it?
	if (mreq_->size() > msize) {
		sub = mreq_->extract(msize);		
	}
	else {
		sub = mreq_;
		MessageBlock* mb = sub->cont();
		size_t left = msize - mreq_->size();
		while (left > 0) {
			if (left < mb->size()) {
				mreq_ = new MessageRequest(mb->dblock_);
				break;
			}
			else if (left == mb->size()) {
				mreq_ = new MessageRequest(mb->cont());
				mb->cont(NULL);
			}
			else {
				left -= mb->size();
				continue;
			}
		}
	}
	

	if (sub != NULL) {
		if ((retval = sched_->on_message(sub)) < 0) {
			SYSLOG_ERROR("%s drop sub message(len=%d, data=%s)",
				name(), (unsigned long)sub->tsize(),
				beyondy::toString((unsigned char *)sub->rptr(), std::min(16, sub->tsize())).c_str(),
				errno);
		}
	}
	else {
		retval = -1;
		SYSLOG_ERROR("%s extract sub message failed: %d", name(), errno);
	}
#endif
	return retval;
}

int Connection::handle_block()
{
	size_t tsize = mreq_->tsize();
	int retval = 0;

	// check whether there is one full message
	while (tsize >= proto_->head_size()) {
		size_t msize = proto_->calc_size(*mreq_);
		if ((ssize_t)msize < 0) {
			SYSLOG_ERROR("%s invalid message(len=%lu, data=%s): %m",
				name(), (unsigned long)msize,
				beyondy::toString(mreq_->rptr(), std::min(size_t(16), msize)).c_str());
			retval = -1;
		}
		else if (msize < tsize) {
			if ((retval = split_block(msize)) < 0)
				break;
			tsize -= msize;
			continue;
		}
		else if (msize == tsize) {
			// exactly one message
			if ((retval = sched_->on_request(mreq_)) < 0) {
				// have to drop it
				SYSLOG_ERROR("%s drop message(len=%lu, data=%s): %m",
					name(), (unsigned long)msize,
					beyondy::toString(mreq_->rptr(), std::min(size_t(16), msize)).c_str());
				delete mreq_;
			}
			
			mbr_ = mreq_ = NULL;
			break;
		}
		else {
			// not enough for one message
			// TODO: adjust???
			retval = 0;
			break;
		}
	}

	return retval;
}

#define RR_MAY_MORE		2
#define RR_CLOSED_BY_PEER	1
#define RR_NO_MORE		0
#define RR_ERROR		-1

int Connection::do_read()
{
	gettimeofday(&mreq_->tr_last_, NULL);
	++mreq_->r_cnt_;

	size_t space = mbr_->space();
	if (space < 1) {
		// TODO: cont-size should come from config
		size_t csize = 8192;
		MessageBlock* nmb;

		try {
			nmb = new MessageBlock(csize);
		}
		catch (std::exception& ex) {
			SYSLOG_ERROR("%s failed to allocate cont block by except: %s", name(), ex.what());
			return RR_ERROR;
		}
		
		if (nmb == NULL) {
			SYSLOG_ERROR("%s failed to allocate cont block: %m", name());
			return RR_ERROR;
		}

		mbr_->cont(nmb);
		mbr_ = nmb;

		space = mbr_->space();
	}

	int retval = RR_NO_MORE;
	ssize_t rlen = recv(fd(), mbr_->wptr(), space, 0);
	if (rlen > 0) {
		if ((size_t)rlen == space) {
			// exactly: process and check again
			retval = RR_MAY_MORE;
		}

		mbr_->wptr(rlen);
	}
	else if (rlen == 0) {
		SYSLOG_ERROR("%s is closed by peer.", name());
		retval = RR_CLOSED_BY_PEER;
	}
	else {
		// sothing wrong
		SYSLOG_ERROR("%s read failed: %m", name());
		retval = RR_ERROR;
	}

	return retval;
}

int Connection::on_readable()
{
	while (1) {
		if (mreq_ == NULL && init_block() < 0) {
			return -1;
		}

		int rstat, retval;
		if ((rstat = do_read()) == RR_ERROR) {
			return -1;
		}

		if ((retval = handle_block()) < 0) {
			return -1;
		}

		if (rstat == RR_MAY_MORE) {
			continue;
		}
		else if (rstat == RR_CLOSED_BY_PEER) {
			return -1;
		}
		else {
			return 0;
		}
	}

	// can not reach here
	assert(0);
	return 0;	
}

int Connection::do_send()
{
	while (mbs_ != NULL) {
		if (mbs_->size() > 0) {
			ssize_t ws = send(fd(), mbs_->rptr(), mbs_->size(), 0);
			if (ws == (ssize_t)-1) {
				SYSLOG_ERROR("%s write %lu failed: %m", name(), (unsigned long)mbs_->size());
				return -1;
			}
			else if ((size_t)ws < mbs_->size()) {
				mbs_->rptr(ws); // TODO: impossible ws overflow?
				return 1;
			}
		}

		if ((mbs_ = mbs_->cont()) == NULL) {
			// all done

			delete mrsp_;
			mbs_ = NULL;
			mrsp_ = NULL;
			break;
		}
	}

	return 0;
}

int Connection::on_writable()
{
	beyondy::auto_lock<typeof outlock_> al(outlock_);
	int retval;
	
	if (mrsp_ == NULL) {
		if ((mrsp_ = outq_.get(0)) == NULL) {
			// should not have such case
			SYSLOG_ERROR("%s is writable while outQ is empty", name());
			return 0;
		}

		mbs_ = mrsp_;
	}

	while (1) {
		if ((retval = do_send()) == 0) {
			// done all
			if ((mrsp_ = outq_.get(0)) != NULL) {
				mbs_ = mrsp_;
				continue;
			}

			return 0;
		}
		else if (retval == 1) {
			// do partial, wait next time
			return 1;
		}
		else {
			// something wrong
			return -1;
		}
	}

	return 0;
}

int Connection::on_error()
{
	return -1;
}

const char* Connection::name() const
{
	return name_;
}

int Connection::idle(const struct timeval& tnow)
{
	// TODO:
	return 0;
}

int Connection::response(MessageResponse* rsp)
{
	beyondy::auto_lock<typeof outlock_> al(outlock_);
	int retval;

	if (outq_.empty() && mrsp_ == NULL) {
		// try send as can as possible first
		// when not all, put rest on the queue
		mrsp_ = rsp;
		mbs_ = mrsp_;

		if ((retval = do_send()) == 0) {
			// all done
			return 0;
		}
		else if (retval == 1) {
			// not all done
			return 0;
		}

		// something wrong
		return -1;
	}
	else {
		// someone are in responing
		// wait in out Q		
		outq_.put(rsp, 1000); // TODO: timeout???
	}

	return 0;
}

