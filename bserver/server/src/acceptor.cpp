/* acceptor.cpp
 *
**/
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "beyondy/xbs_socket.h"
#include "beyondy/xbs_naddr.h"
#include "acceptor.h"
#include "log.h"

Acceptor::Acceptor(const char* addr, protocol* proto, schedule* sched, EventManager* emgr)
	: pollable(-1), proto_(proto), sched_(sched), emgr_(emgr), addr_(addr)
{
	int _fd = beyondy::XbsServer(addr, 8192, O_NONBLOCK);
	if (_fd < 0) {
		char buf[512];
		snprintf(buf, sizeof buf, "listen at %s failed: %m", addr);

		SYSLOG_ERROR("%s", buf);
		throw std::runtime_error(buf);
	}

	socklen_t alen = sizeof(naddr_);
	getsockname(_fd, &naddr_, &alen);
	fd(_fd);

	SYSLOG_INFO("listen at %s (%s) ok", addr, ({
		char nbuf[256]; 
		beyondy::XbsNaddr2p(&naddr_, SOCK_STREAM, 0, nbuf, sizeof(nbuf)); nbuf;}) );
}

Acceptor::~Acceptor()
{
	// TODO: clean
}

int Acceptor::on_readable()
{
	int nfd;

	while (true) {
		struct sockaddr addr;
		socklen_t alen = sizeof addr;
		if ((nfd = accept(fd(), &addr, &alen)) >= 0) {
			Connection* conn = create(nfd);
			if (conn != NULL) {
				SYSLOG_INFO("acceptor %s accept connection: %s", name(), conn->name());
				emgr_->add_poller(conn, EPOLLIN);
			}
			else {
				SYSLOG_ERROR("failed to create connection for #%d from acceptor %s", nfd, name());
				close(nfd);
			}
		}
		else if (errno == EINTR) {
			continue;
		}
		else if (errno == EWOULDBLOCK) {
			break;
		}
		else {
			SYSLOG_ERROR("acceptor %s accept failed: %m", name());
			break;
		}
	} 

	return 0;
}

Connection* Acceptor::create(int fd)
{
	try {
		Connection* conn = new PassiveConnection(fd, emgr_->next_flow(), proto_, sched_, this);
		return conn;
	}
	catch (std::exception& ex) {
		SYSLOG_ERROR("acceptor %s create connection failed: %s", name(), ex.what());
	}
	catch (...) {
		SYSLOG_ERROR("acceptor %s create connection failed: %m", name());
	}
	
	return NULL;
}
	
void Acceptor::destory(Connection* conn)
{
	SYSLOG_INFO("acceptor %s destroy connection: %s", name(), conn->name());
	try {
		PassiveConnection* pc = dynamic_cast<PassiveConnection *>(conn);
		assert(pc != NULL);
		delete pc;
	}
	catch (std::exception& ex) {
		SYSLOG_ERROR("destory %s failed: %s", conn->name(), ex.what());	
	}

	return;
}

void PassiveConnection::destory()
{
	assert(acceptor_ != NULL);
	acceptor_->destory(this);
}

