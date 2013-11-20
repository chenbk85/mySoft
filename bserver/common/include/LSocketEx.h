#ifndef __LSOCKETEX__H
#define __LSOCKETEX__H

#include <fcntl.h>
#include <sstream>
#include <beyondy/timedout_countdown.hpp>
#include <beyondy/xbs_socket.h>
#include <beyondy/xbs_io.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "ProtoErr.h"

class CSocketEx
{
public:
	CSocketEx (const char* addr, int port, long msTimedout)
	{
		char url[256] = {0};
		uint32_t ip = 0;
		if (!hostServ(addr, NULL, AF_INET, SOCK_STREAM, ip))
		{
		    std::stringstream msg;
		    msg << "Failed to call hostServ(), errno=" << PROTO_ERR_CONNECT;
		    throw std::runtime_error(msg.str());
		}

		sprintf(url, "%u.%u.%u.%u:%u/tcp", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff, port);

		//m_socket = beyondy::XbsClient(url, 0, msTimedout);
		m_socket = beyondy::XbsClient(url, O_NONBLOCK, msTimedout);
		if (m_socket < 0)
		{
			std::stringstream msg;
			msg << "Failed to call socket(), errno=" << PROTO_ERR_CONNECT;
			throw std::runtime_error(msg.str());
		}

		int32_t optval = 1;
		socklen_t optlen = sizeof(int32_t);
		if (setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) != 0)
		{
			Close();
			std::stringstream msg;
			msg << "Failed to call setsockopt() for SO_KEEPALIVE, errno=" << PROTO_ERR_SOCKET;
			throw std::runtime_error(msg.str());
		}

		optval = 60;
		if (setsockopt(m_socket, SOL_TCP, TCP_KEEPIDLE, &optval, optlen) != 0)
		{
			Close();
			std::stringstream msg;
			msg << "Failed to call setsockopt() for TCP_KEEPIDLE, errno=" << PROTO_ERR_SOCKET;
			throw std::runtime_error(msg.str());
		}

		optval = 10;
		if (setsockopt(m_socket, SOL_TCP, TCP_KEEPINTVL, &optval, optlen) != 0)
		{
			Close();
			std::stringstream msg;
			msg << "Failed to call setsockopt() for TCP_KEEPINTVL, errno=" << PROTO_ERR_SOCKET;
			throw std::runtime_error(msg.str());
		}

		optval = 5;
		if (setsockopt(m_socket, SOL_TCP, TCP_KEEPCNT, &optval, optlen) != 0)
		{
			Close();
			std::stringstream msg;
			msg << "Failed to call setsockopt() for TCP_KEEPCNT, errno=" << PROTO_ERR_SOCKET;
			throw std::runtime_error(msg.str());
		}

	}

	virtual ~CSocketEx ()
	{
		Close();
	}

public:
	void Close ()
	{
		close(m_socket);
		m_socket = -1;
	}

	int SafeWrite (const char* sendBuf, int sendLen, long timeOut)
	{
		if (timeOut == 0)
		{
			return PROTO_ERR_TIMEOUT;
		}

		beyondy::TimedoutCountdown toCountdown(timeOut);
		ssize_t wlen = 0;
		int try_time = 0;
		while (wlen < sendLen)
		{
			ssize_t send_size = 0;
			send_size = beyondy::XbsWrite(m_socket, sendBuf + wlen, sendLen - wlen, toCountdown.Update());

			if (send_size < 0 && EINTR == errno && try_time < 3)
			{
				try_time++;
				continue;
			}
			try_time = 0;

			if (send_size < 0)
			{
				if (EPIPE == errno)
				{
					return PROTO_ERR_DISCONNECT_BY_PEER;
				}
				else if (errno == ETIMEDOUT) 
				{
					return PROTO_ERR_TIMEOUT;
				}
				else 
				{
					return PROTO_ERR_SOCKET;
				}
			}

			wlen += send_size;
		}

		return 0;
	}



	int SafeRead (char* recvBuf, int recvLen, long timeOut)
	{
		if ( timeOut == 0 )
		{
			return PROTO_ERR_TIMEOUT;
		}

		beyondy::TimedoutCountdown toCountdown(timeOut);

		ssize_t rlen = 0;

		int try_time = 0;
		while (rlen < recvLen)
		{
			ssize_t recv_size = 0;
//			if (beyondy::XbsWaitReadable(m_socket, toCountdown.Update()) < 0)
//			{
//				return PROTO_ERR_TIMEOUT;
//			}

			recv_size = beyondy::XbsReadN(m_socket, recvBuf + rlen, recvLen - rlen, toCountdown.Update());
			if (recv_size < 0 && EINTR == errno && try_time < 3)
			{
				try_time++;
				continue;
			}
			try_time = 0;

			if (recv_size == 0) {
				return PROTO_ERR_RESET_BY_PEER;
			}
			
			if (recv_size < 0)
			{				
				if (errno == ETIMEDOUT) {
					return PROTO_ERR_TIMEOUT;
				}				
				return PROTO_ERR_SOCKET;
			}
			rlen += recv_size;
		}
		return 0;
	}

private:
	bool hostServ (const char* host, const char* serv, int family, int socktype, uint32_t& ip)
	{
		int n;
		struct addrinfo hints, *res = NULL;

		bzero(&hints, sizeof(struct addrinfo));
		hints.ai_flags = AI_PASSIVE;
		hints.ai_family = family;
		hints.ai_socktype = socktype;

		sockaddr_in* sa = NULL;

		if ((n = getaddrinfo(host, serv, &hints, &res)) == 0)
		{
			sa = (struct sockaddr_in*)res->ai_addr;
			ip = sa->sin_addr.s_addr;
			if (res != NULL)
			{
				freeaddrinfo(res);
			}
			return true;
		}
		return false;
	}

private:
	int32_t  m_socket;
};
#endif //__LSOCKETEX__H
