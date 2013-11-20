#ifndef __PROTOEX__H
#define __PROTOEX__H

#include "ProtoErr.h"
#include "mstar_protocol.h"

#if defined WIN32
#include "SocketEx.h"
#else
#include "LSocketEx.h"
#include "log.h"
#endif

#define IP_INFO_LEN 32
static uint64_t visit_timeout = 5000;

template <typename T_ProtoClass>
class ProtoEx
{
public:
    ProtoEx ();
    virtual ~ProtoEx ();
    ProtoEx (const char* addr);

public:
    int AddServer (const char* addr);
    int AddServer (const char* addr, int port);
    template <typename T_Req, typename T_Res>
    int Request (unsigned int cmd, const T_Req& op, int reqSize, T_Res& ip, long timeOut);
	char * GetIp();
//add by jackie 2010-10-27
	void UpdateVisitTime();
	//time_t GetVisitTime();
	void CheckVisitTime(uint64_t timeout);
// add end
private:
    int _ConnectServer (const char* addr, int port, long msTimedout);
private:
    std::string m_serverAddr;
    int m_serverPort;
    unsigned int m_sync;
    CSocketEx* m_link;
    char m_newBuf[sizeof(CSocketEx)];

// add by jackie 2010-10-27
// Description: recently visit time
	uint64_t m_visit_time;
// add end
};

template <typename T_ProtoClass>
ProtoEx<T_ProtoClass>::ProtoEx ()
{
    m_link = NULL;
    m_serverAddr = "";
    m_serverPort = 0;
    m_sync = 0;
    UpdateVisitTime();
}

template <typename T_ProtoClass>
ProtoEx<T_ProtoClass>::ProtoEx (const char* addr):m_sync(0),m_link(NULL)
{
	UpdateVisitTime();
    int ret;
    if ((ret = AddServer(addr)) != 0)
    {
        std::stringstream msg;
        msg << "Failed to connect server, errno=" << ret;
        throw std::runtime_error(msg.str());
    }
}


template <typename T_ProtoClass>
ProtoEx<T_ProtoClass>::~ProtoEx ()
{
	if ( m_link )
	{
		m_link->Close();
		m_link = NULL;
	}
}

template <typename T_ProtoClass>
int ProtoEx<T_ProtoClass>::AddServer (const char* addr)
{
    std::string::size_type index1;
    std::string::size_type index2;
    std::string serverAddr;
    int port;
    std::string str(addr);

// add by jackie 2010-04-06
// Description: support inet@domain:port/tcp
//#if defined WIN32
    std::string::size_type index3;
    index3 = str.find("@");
    if(index3 != std::string::npos)
    {
          if(str.substr(0, index3).compare("inet") != 0)
          {
		  return PROTO_ERR_INVALID_ADDR;
          }
	   else
	   {
             str = str.substr(index3 + 1,  str.size() - index3);
	   }
    }
//#endif
// add end

    index1 = str.find(":");
    if (index1 == std::string::npos)
    {
        return PROTO_ERR_INVALID_ADDR;
    }
    else
    {
        serverAddr = str.substr(0, index1);
    }

    index2 = str.find("/");
    if (index2 != std::string::npos)
    {
        if (str.substr(index2 + 1, str.size() - index2).compare("tcp") != 0)
        {
            return PROTO_ERR_INVALID_ADDR;
        }
        port = atoi(str.substr(index1 + 1, index2 - index1).c_str());
    }
    else
    {
         port = atoi(str.substr(index1 + 1, str.size() - index1).c_str());
    }

    return AddServer(serverAddr.c_str(), port);
}

template <typename T_ProtoClass>
int ProtoEx<T_ProtoClass>::AddServer (const char* addr, int port)
{
    m_serverAddr = addr;
    m_serverPort = port;
    return 0;
}

#ifdef WIN32
extern bool write_log_(const char *log, int log_len);
#endif

template <typename T_ProtoClass>
int ProtoEx<T_ProtoClass>::_ConnectServer (const char* addr, int port, long msTimedout)
{
	long msConnetTimeout=msTimedout/5;
	if (msConnetTimeout<1000)
	{
		msConnetTimeout=1000;
	}
    try
    {
#if defined WIN32
		char buf[512] = {0};
		sprintf(buf, "%s:%d _ConnectServer, addr:%s, port:%d\n",__FILE__, __LINE__,addr, port);
		writeLog(INFO_LEVEL, buf);

		char logbuf[512];
		int result = sprintf(logbuf, "%s:%d, %s: _ConnectServer, addr:%s, port:%d.\r\n",
			__FILE__, __LINE__, __FUNCTION__, addr, port);
		write_log_(logbuf, result);
#else
		syslog()->debug("%s:%d _ConnectServer, addr:%s, port:%d\n",
		__FILE__, __LINE__,addr, port);
#endif
        m_link = new(m_newBuf) CSocketEx(addr, port, msConnetTimeout);
    }
	catch (std::exception &ex)
    {
#if defined WIN32
		char buf[512] = {0};
		int retCode = PROTO_ERR_CONNECT;
		sprintf(buf, "%s:%d _ConnectServer error, addr:%s, port:%d, error message: %s, return code:%d\n",__FILE__, __LINE__,addr, port, ex.what(), retCode);
		writeLog(ERROR_LEVEL, buf);

            char logbuf[512];
            int result = sprintf(logbuf, "%s:%d, %s: _ConnectServer error, addr:%s, port:%d, error message: %s.\r\n",
                __FILE__, __LINE__, __FUNCTION__, addr, port, ex.what());
            write_log_(logbuf, result);
#else
		syslog()->error("%s:%d _ConnectServer error, addr:%s, port:%d, error message: %s, return code:%d\n",
		__FILE__, __LINE__,addr, port, ex.what(), PROTO_ERR_CONNECT);
#endif

        return PROTO_ERR_CONNECT;
    }

    return 0;
}

template <typename T_ProtoClass>
template <typename T_Req, typename T_Res>
int ProtoEx<T_ProtoClass>::Request (unsigned int cmd, const T_Req& req, int reqSize, T_Res& res, long timeOut)
{
    beyondy::TimedoutCountdown countdown(timeOut);
    T_ProtoClass op(reqSize + MSTAR_HEAD_SIZE);

    if (op.prepare_encode() < 0)
    {
        return PROTO_ERR_ENCODE;
    }

    if (op.encode(req) < 0)
    {
        return PROTO_ERR_ENCODE;
    }

    op.head().cmd_ = cmd;
    op.head().ver_ = MP_VERSION_1;
    op.head().syn_ = m_sync;

	if (op.end_encode() < 0 )
	{
        return PROTO_ERR_ENCODE;
	}
	CheckVisitTime(visit_timeout);

    if (m_link == NULL)
    {
		if (_ConnectServer(m_serverAddr.c_str(), m_serverPort, countdown.Update()) != 0)
        {
 #if defined WIN32
       		int retCode = PROTO_ERR_CONNECT;
			char buf[512] = {0};
			sprintf(buf, "%s:%d _ConnectServer error,  socket count: %d, return code:%d\n",__FILE__, __LINE__, CSocketEx::count, retCode);
			writeLog(ERROR_LEVEL,buf);
            char logbuf[512];
            int result = sprintf(logbuf, "%s:%d, %s: _ConnectServer error,  socket count: %d.\r\n",
                __FILE__, __LINE__, __FUNCTION__, CSocketEx::count);
            write_log_(logbuf, result);
#endif

            return PROTO_ERR_CONNECT;
        }
    }

    //send req
    int ret = m_link->SafeWrite((char*)op.dblock().data(), (int)op.size(), countdown.Update());
    if (ret != 0)
    {
#if defined WIN32
		char buf[512] = {0};
		sprintf(buf, "%s:%d m_link->SafeWrite (send req) error, socket count: %d\n",__FILE__, __LINE__, CSocketEx::count);
   		writeLog(ERROR_LEVEL, buf);

        char logbuf[512];
        int result = sprintf(logbuf, "%s:%d, %s: m_link->SafeWrite (send req) error, socket count: %d, return code: %d.\r\n",
            __FILE__, __LINE__, __FUNCTION__, CSocketEx::count, ret);
        write_log_(logbuf, result);
#endif

        m_link->Close();
        m_link = NULL;
        return ret;
    }

    //receive head
    mstar_proto_head head;
    unsigned char hbuf[MSTAR_HEAD_SIZE];
    ret = m_link->SafeRead((char*)hbuf, MSTAR_HEAD_SIZE, countdown.Update());
    if (ret != 0)
    {
#if defined WIN32
		char buf[512] = {0};
		sprintf(buf, "%s:%d m_link->SafeRead (receive head) error, socket count: %d\n",__FILE__, __LINE__, CSocketEx::count);
		writeLog(ERROR_LEVEL, buf);

        char logbuf[512];
        int result = sprintf(logbuf, "%s:%d, %s: m_link->SafeRead (receive head) error, socket count: %d, return code: %d.\r\n",
            __FILE__, __LINE__, __FUNCTION__, CSocketEx::count, ret);
        write_log_(logbuf, result);
#endif

		m_link->Close();
        m_link = NULL;
        return ret;
    }

    //receive body
	head.ntoh(hbuf, MSTAR_HEAD_SIZE);
    if (head.len_ <= MSTAR_HEAD_SIZE)
    {
#if defined WIN32
   		int retCode = PROTO_ERR_RECV_HEAD;
		char buf[512] = {0};
		sprintf(buf, "%s:%d m_link->SafeRead (receive body) error, socket count: %d, return code:%d\n",__FILE__, __LINE__, CSocketEx::count, retCode);
		writeLog(ERROR_LEVEL, buf);

        char logbuf[512];
        int result = sprintf(logbuf, "%s:%d, %s: m_link->SafeRead (receive body) error, socket count: %d, head.len_: %u.\r\n",
            __FILE__, __LINE__, __FUNCTION__, CSocketEx::count, head.len_);
        write_log_(logbuf, result);
#endif

        m_link->Close();
        m_link = NULL;
        return PROTO_ERR_RECV_HEAD;
    }

    unsigned char* recvBuf = new (std::nothrow) unsigned char[head.len_];
    if (recvBuf == NULL)
    {
#if defined WIN32
   		int retCode = PROTO_ERR_NO_MEM;
		char buf[512] = {0};
		sprintf(buf, "%s:%d m_link->SafeRead (no memory) error, socket count: %d, return code:%d\n",__FILE__, __LINE__, CSocketEx::count, retCode);
		writeLog(ERROR_LEVEL, buf);

        char logbuf[512];
        int result = sprintf(logbuf, "%s:%d, %s: m_link->SafeRead (no memory to receive body data) error, socket count: %d.\r\n",
            __FILE__, __LINE__, __FUNCTION__, CSocketEx::count);
        write_log_(logbuf, result);
#endif

		m_link->Close();
        m_link = NULL;
        return PROTO_ERR_NO_MEM;
    }

    memcpy(recvBuf, hbuf, MSTAR_HEAD_SIZE);
    ret = m_link->SafeRead((char*)(recvBuf + MSTAR_HEAD_SIZE), head.len_ - MSTAR_HEAD_SIZE, countdown.Update());
    if (ret != 0)
    {
#if defined WIN32
		char buf[512] = {0};
		sprintf(buf, "%s:%d m_link->SafeRead (receive data) error, socket count: %d, return code:%d \n",__FILE__, __LINE__, CSocketEx::count, ret);
		writeLog(ERROR_LEVEL, buf);

        char logbuf[512];
        int result = sprintf(logbuf, "%s:%d, %s: m_link->SafeRead (receive data) error, socket count: %d, return code:%d.\r\n",
            __FILE__, __LINE__, __FUNCTION__, CSocketEx::count, ret);
        write_log_(logbuf, result);
#endif

		m_link->Close();
        m_link = NULL;
        delete [] recvBuf;
        return ret;
    }

    T_ProtoClass ip((unsigned char*)recvBuf, head.len_, DECODE);

    if (ip.head().cmd_ != cmd + 1 || ip.head().ack_ != m_sync)
    {
#if defined WIN32
		int retCode = PROTO_ERR_MSG_SYNC;
		char buf[512] = {0};
		sprintf(buf, "%s:%d message error, socket count = %d, return code:%d\n",__FILE__, __LINE__, CSocketEx::count, retCode);
		writeLog(ERROR_LEVEL, buf);

        char logbuf[512];
        int result = sprintf(logbuf, "%s:%d, %s: socket count = %d, cmd %u/%u or sync %u/ack %u mismatch.\r\n",
            __FILE__, __LINE__, __FUNCTION__, CSocketEx::count, cmd + 1, ip.head().cmd_, m_sync, ip.head().ack_);
        write_log_(logbuf, result);
#endif

		m_link->Close();
        m_link = NULL;
		ip.auto_delete(true);
        return PROTO_ERR_MSG_SYNC;
    }

    if (ip.decode(res) < 0)
    {
 #if defined WIN32
		int retCode = PROTO_ERR_DECODE;
		char buf[512] = {0};
		sprintf(buf, "%s:%d res message decode error, socket count = %d, return code:%d\n",__FILE__, __LINE__, CSocketEx::count, retCode);
		writeLog(ERROR_LEVEL,buf);

        char logbuf[512];
        int result = sprintf(logbuf, "%s:%d, %s: res message decode error, socket count = %d.\r\n",
            __FILE__, __LINE__, __FUNCTION__, CSocketEx::count);
        write_log_(logbuf, result);
#endif

		m_link->Close();
        m_link = NULL;
        ip.auto_delete(true);
        return PROTO_ERR_DECODE;
    }

    res.dblock = ip.dblock();
    res.dblock.auto_delete(true);
    m_sync++;
    UpdateVisitTime();
    return 0;
}

// add by jackie 2010-10-27
template <typename T_ProtoClass>
void ProtoEx<T_ProtoClass>::UpdateVisitTime ()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	m_visit_time = ((uint64_t)tv.tv_sec * 1000 + tv.tv_usec/1000);
}

template <typename T_ProtoClass>
void ProtoEx<T_ProtoClass>::CheckVisitTime (uint64_t timeout)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t cur_time = ((uint64_t)tv.tv_sec * 1000 + tv.tv_usec/1000);

	uint64_t pass_time = cur_time - m_visit_time;
	if(pass_time > timeout)
	{
		if(m_link)
		{
			m_link->Close();
			m_link = NULL;
		}
	}
}

// add end
template <typename T_ProtoClass>
char* ProtoEx<T_ProtoClass>::GetIp ()
{
	char *ip = (char*)malloc(IP_INFO_LEN);
	sprintf(ip, "%s:%d/tcp", m_serverAddr.c_str(),m_serverPort);

	return  ip;
}
#endif //__PROTOEX__H

