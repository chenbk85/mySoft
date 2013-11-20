#ifndef __SOCKETEX__H
#define __SOCKETEX__H

#include <time.h>
#include <Winsock2.h>
#include <sstream>
#include "ProtoErr.h"
#include "beyondy\timedout_countdown.hpp"
#include "thread_lock.h"

static  thread_lock g_lock;
enum LOG_LEVEL 
{
	NO_LOG = 0,
	INFO_LEVEL = 1,
	WARN_LEVEL = 2,	
	DEBUG_LEVEL = 3,
	ERROR_LEVEL = 4,
};

static std::string log_file_ = "c:\\err.log";
static int log_level_ = ERROR_LEVEL;

static void setLogParameter(int log_level, std::string log_file)
{
	if(log_level < NO_LOG)
	{
		log_level_ = NO_LOG;
	}	
	else if(log_level > ERROR_LEVEL) 
	{
		log_level_ = ERROR_LEVEL;
	}	
	else
	{
		log_level_ = log_level;
	}

	log_file_ = log_file;
}

static bool writeLog(int level, std::string strContent)
{
	auto_lock al(g_lock);

	if(level < log_level_ || log_level_ == NO_LOG ) return true;
	if(log_file_.size() == 0) return true;
		
    	time_t timer;
	struct tm* tb;
	timer = time(NULL);
	tb = localtime(&timer);

	std::string strTime = asctime(tb);
	DWORD thread_id = GetCurrentThreadId();
	
	std::stringstream msg;
	msg <<thread_id<<" "<<tb->tm_mday<<"/"<<tb->tm_mon+1<<"/"<<tb->tm_year+1900<<" "<<tb->tm_hour<<":"<<tb->tm_min<<":"<<tb->tm_sec<<" ";

	strContent = msg.str() + " " + strContent;
	
	bool bResult = false;
	FILE* fp = fopen( log_file_.c_str(), "a");
	if (fp)
	{
		int nFileLen = (int)strContent.size();
		if (nFileLen ==0)
		{
			bResult = true;
		}
		else if (fwrite(strContent.c_str(), nFileLen, 1, fp) == 1)
		{
			bResult = true;
		}
		fclose(fp);
	}
	return bResult;
}

class CSocketEx
{
public:
	static int count;

public:
    CSocketEx (const char* addr, int port, long msTimedout):m_init(false)
    {
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;
        wVersionRequested = MAKEWORD(2, 2);

        err = WSAStartup(wVersionRequested, &wsaData );
        if (err != 0)
        {
            std::stringstream msg;
		msg << "Failed to call WSAStartup(), errno=" << err<<", count:" <<count<<", m_socket:"<<(int)m_socket<<", addr:"<<addr<<", port:"<<port<<std::endl;
            throw std::runtime_error(msg.str());
        }

        m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_socket == INVALID_SOCKET)
        {
            std::stringstream msg;
            err = WSAGetLastError();
		msg << "Failed to call socket(), errno=" << err<<" ,count:" <<count<<" , m_socket:"<<(int)m_socket<<", addr:"<<addr<<", port:"<<port<<std::endl;
            WSACleanup();
            throw std::runtime_error(msg.str());
        }

        //-------------------------
        // Set the socket I/O mode: In this case FIONBIO
        // enables or disables the blocking mode for the 
        // socket based on the numerical value of iMode.
        // If mode = 0, blocking is enabled; 
        // If mode != 0, non-blocking mode is enabled.
        u_long mode = 1;
        if (SOCKET_ERROR == ioctlsocket(m_socket, FIONBIO, &mode))
        {
            std::stringstream msg;
            err = WSAGetLastError();
            msg << "Failed to call socket(), errno=" << err<<" ,count:" <<count<<" , m_socket:"<<(int)m_socket<<", addr:"<<addr<<", port:"<<port<<std::endl;
            closesocket(m_socket);
            WSACleanup();
            throw std::runtime_error(msg.str());
        }

        hostent* pHostent = gethostbyname(addr);
        if (pHostent == NULL)
        {   
            std::stringstream msg;
            err = WSAGetLastError();
            msg << "Failed to call gethostbyname(), errno=" << err<<" ,count:" <<count<<" , m_socket:"<<(int)m_socket<<", addr:"<<addr<<", port:"<<port<<std::endl;
            closesocket(m_socket);
            WSACleanup();
            throw std::runtime_error(msg.str());
        }
      
        struct sockaddr_in _addr;
        memset((void*)&_addr, 0, sizeof(_addr));
        int nAdapter=0;
        //for  (nAdapter=0; he.h_addr_list[nAdapter]; nAdapter++)   
        {   
            memcpy(&_addr.sin_addr.s_addr, pHostent->h_addr_list[nAdapter], pHostent->h_length);        
        }

        _addr.sin_family = AF_INET;
        _addr.sin_port = htons(port);
        if (SOCKET_ERROR == connect(m_socket, (SOCKADDR*)&_addr, sizeof(SOCKADDR_IN)))
        {
            std::stringstream msg;
            err = WSAGetLastError();
            if (WSAEWOULDBLOCK == err)
            {
                timeval timeOut;
                int ret;

                timeOut.tv_sec = msTimedout / 1000;
                timeOut.tv_usec = (msTimedout % 1000) * 1000;
                fd_set m_fdwrite;
                FD_ZERO(&m_fdwrite);
                FD_SET(m_socket, &m_fdwrite);
                ret = select(0, NULL, &m_fdwrite, NULL, &timeOut);
                if (ret == 1)
                {
                    m_init = true;
					count++;
					char buf[512] = {0};
					sprintf(buf, "%s:%d connect success, count: %d, m_socket: %d\n, addr: %s, port: %d\n",__FILE__, __LINE__, count, (int)m_socket, addr, port);
					writeLog(ERROR_LEVEL, buf);
                    return;
                }
                else if (ret == 0)
                {
			msg << "Failed to call socket(), wait writeable event timeout" <<" ,count:" <<count<<" , m_socket:"<<(int)m_socket<<std::endl;
                }
                else //SOCKET_ERROR
                {
                    err = WSAGetLastError();
                    msg << "Failed to call select(), errno=" << err<<" ,count:" <<count<<" , m_socket:"<<(int)m_socket<<", addr:"<<addr<<", port:"<<port<<std::endl;
                }
            }
            else
            {
                msg << "Failed to call socket(), errno=" << err<<" ,count:" <<count<<" , m_socket:"<<(int)m_socket<<", addr:"<<addr<<", port:"<<port<<std::endl;
            }

            closesocket(m_socket);		
            WSACleanup();
            throw std::runtime_error(msg.str());
        }
    }

    virtual ~CSocketEx ()
    {
        Close();
    }

    void Close ()
    {
        if (m_init)
        {		
			count--;
			char buf[512] = {0};
			sprintf(buf, "%s:%d connect close success, count: %d, m_socket: %d\n",__FILE__, __LINE__, count, (int)m_socket);
			writeLog(ERROR_LEVEL, buf);

			closesocket(m_socket);
            WSACleanup();
            m_socket = -1;
        }
    }

    int SafeWrite (const char* sendBuf, int sendLen, long timeOut)
    {
        if (timeOut == 0)
        {
   			char buf[512] = {0};
			int retCode = PROTO_ERR_TIMEOUT;
			sprintf(buf, "%s:%d SafeWrite error, m_socket:%d, retCode: %d \n",__FILE__, __LINE__, (int)m_socket,  retCode );
			writeLog(ERROR_LEVEL, buf);
			
            return PROTO_ERR_TIMEOUT;
        }

        beyondy::TimedoutCountdown time(timeOut);
        int sentLen = 0;
        int tryTime = 0;
        int sendingSize = 0;
        int ret;

        while (sentLen < sendLen)
        {
            ret = Write(sendBuf + sentLen, sendLen - sentLen, sendingSize, time.Update());

            if (ret == PROTO_ERR_SOCKET && WSAEINTR == WSAGetLastError() && tryTime < 3)
            {
            		char buf[512] = {0};
					sprintf(buf, "%s:%d tryTime = %d, SafeWrite Write error, m_socket:%d, ret: %d \n",__FILE__, __LINE__, tryTime, (int)m_socket,  ret );
					writeLog(ERROR_LEVEL, buf);
            		
                	tryTime++;
                	continue;
            }

            tryTime = 0;

            if (ret != 0)
            {
            		char buf[512] = {0};
					sprintf(buf, "%s:%d SafeWrite Write error, m_socket: %d, ret: %d \n",__FILE__, __LINE__,(int)m_socket,  ret);
					writeLog(ERROR_LEVEL, buf);
                    return ret;
            }
            sentLen += sendingSize;
        }

        return 0;
    }

    int WaitWriteable (long timeOut)
    {
        if (timeOut == 0)
        {
        	char buf[512] = {0};
			int retCode = PROTO_ERR_TIMEOUT;
			sprintf(buf, "%s:%d WaitWriteable error, m_socket:%d, return code: %d \n",__FILE__, __LINE__, (int)m_socket,  retCode );
			writeLog(ERROR_LEVEL, buf);
    		
            return PROTO_ERR_TIMEOUT;
        }
        int ret;
        fd_set m_fdwrite;
        FD_ZERO(&m_fdwrite);
        FD_SET(m_socket, &m_fdwrite);

        if (timeOut < 0)
        {
            ret = select(0, NULL, &m_fdwrite, NULL, NULL);
        }
        else
        {
            timeval tv;
            tv.tv_sec = timeOut / 1000;
            tv.tv_usec = (timeOut % 1000) *1000;
            ret = select(0, NULL, &m_fdwrite, NULL, &tv);
        }

        if (ret == 0) //time out
        {
			char buf[512] = {0};
			int retCode = PROTO_ERR_TIMEOUT;
			sprintf(buf, "%s:%d WaitWriteable timeout, m_socket = %d, return code = %d \n",__FILE__, __LINE__, (int)m_socket,  retCode);
			writeLog(ERROR_LEVEL,  buf);

			return PROTO_ERR_TIMEOUT;
        }
        else if (ret == 1) //there is only fd
        {
            return 0;
        }
        else //SOCKET_ERROR
        {
        	char buf[512] = {0};
			sprintf(buf, "%s:%d WaitWriteable select error, m_socket = %d, ret = %d \n",__FILE__, __LINE__, (int)m_socket, ret);
			writeLog(ERROR_LEVEL, buf);
        		
            return PROTO_ERR_SOCKET;
        }
    }

    int Write (const char* sendBuf, int sendLen, int& sentLen, long timeOut)
    {
        if (timeOut == 0)
        {
            return PROTO_ERR_TIMEOUT;
        }

        int ret;
        int step;
        beyondy::TimedoutCountdown time(timeOut);
        sentLen = 0;
        while (true)
        {
            if (sentLen == sendLen)
            {
                return 0;
            }

            ret = WaitWriteable(time.Update());
            
            if (ret != 0)
            {
				char buf[512] = {0};
				sprintf(buf, "%s:%d write error, m_socket: %d, WaitWriteable return error, ret: %d\n",__FILE__, __LINE__, (int)m_socket,  ret);
				writeLog(ERROR_LEVEL, buf);
		        return ret;
            }

            step = sendLen - sentLen;            
            ret = send(m_socket, sendBuf + sentLen, step, 0);
            
            if (ret == SOCKET_ERROR)
            {
            	int lastError=WSAGetLastError();
	int retCode = PROTO_ERR_SOCKET;
	if (lastError==WSAESHUTDOWN || lastError==WSAENETDOWN 
		|| lastError==WSAEHOSTUNREACH || lastError==WSAECONNRESET
		|| lastError==WSAENETRESET || lastError==WSAECONNABORTED)
	{
		retCode=PROTO_ERR_DISCONNECT_BY_PEER;
	}
				char buf[512] = {0};
				sprintf(buf, "%s:%d write error, m_socket: %d, ret: %d, WSAGetLastError(): %d, sendLen: %d, sentLen: %d, step: %d, return code:%d \n",__FILE__, __LINE__, (int)m_socket, ret, lastError, sendLen, sentLen, step, retCode);
				writeLog(ERROR_LEVEL, buf);
	
		        return retCode;
            }
            sentLen += ret;
        }
        return 0;
    }


    int SafeRead (char* recvBuf, int recvLen, long timeOut)
    {
        if (timeOut == 0)
        {
            return PROTO_ERR_TIMEOUT;
        }

        beyondy::TimedoutCountdown time(timeOut);
        int readLen = 0;
        int tryTime = 0;
        int readingSize = 0;
        int ret;

        while (readLen < recvLen)
        {
            ret = Read(recvBuf + readLen, recvLen - readLen, readingSize, time.Update());

            if (ret == PROTO_ERR_SOCKET && WSAEINTR == WSAGetLastError() && tryTime < 3)
            {
				char buf[512] = {0};
				sprintf(buf, "%s:%d tryTime = %d, SafeRead Read error, m_socket: %d, ret: %d, WSAGetLastError(): %d \n",__FILE__, __LINE__, tryTime, (int)m_socket, ret, (int)WSAGetLastError());
				writeLog(ERROR_LEVEL, buf);
		
				tryTime++;
                continue;
            }

            tryTime = 0;

            if (ret != 0)
            {
				char buf[512] = {0};
				sprintf(buf, "%s:%d SafeRead read error, m_socket: %d, ret: %d, WSAGetLastError(): %d \n",__FILE__, __LINE__, (int)m_socket, ret, (int)WSAGetLastError());
				writeLog(ERROR_LEVEL, buf);
                return ret;
            }
            readLen += readingSize;
        }

        return 0;
    }

    int WaitReadable (long timeOut)
    {
        if (timeOut == 0)
        {
            return PROTO_ERR_TIMEOUT;
        }
        int ret;
        fd_set m_fdread;
        FD_ZERO(&m_fdread);
        FD_SET(m_socket, &m_fdread);

        if (timeOut < 0)
        {
            ret = select(0, &m_fdread, NULL, NULL, NULL);
        }
        else
        {
            timeval tv;
            tv.tv_sec = timeOut / 1000;
            tv.tv_usec = (timeOut % 1000) *1000;
            ret = select(0, &m_fdread, NULL, NULL, &tv);
        }

        if (ret == 0) //time out
        {
			char buf[512] = {0};
			int retCode = PROTO_ERR_TIMEOUT;
			sprintf(buf, "%s:%d WaitReadable select error( time out ), m_socket: %d, ret: %d, WSAGetLastError():%d, return code: %d \n",__FILE__, __LINE__,(int)m_socket, ret, (int)WSAGetLastError(), retCode);
			writeLog(ERROR_LEVEL, buf);
            return PROTO_ERR_TIMEOUT;
        }
        else if (ret == 1) //there is only fd
        {
            return 0;
        }
        else //SOCKET_ERROR
        {
       		char buf[512] = {0};
			int retCode = PROTO_ERR_SOCKET;
			sprintf(buf, "%s:%d WaitReadable select error, m_socket:%d, ret: %d, WSAGetLastError():%d, return code:%d \n",__FILE__, __LINE__, (int)m_socket, ret, (int)WSAGetLastError(), retCode);
			writeLog(ERROR_LEVEL, buf);
            return PROTO_ERR_SOCKET;
        }
    }

    int Read (char* recvBuf, int recvLen, int& readLen, long timeOut)
    {
        if (timeOut == 0)
        {
            return PROTO_ERR_TIMEOUT;
        }

        int ret;
        int step;
        beyondy::TimedoutCountdown time(timeOut);
        readLen = 0;
        while (true)
        {
            if (readLen == recvLen)
            {
                return 0;
            }

            ret = WaitReadable(time.Update());
            
            if (ret != 0)
            {
                return ret;
            }

            step = recvLen - readLen;            
            ret = recv(m_socket, recvBuf + readLen, step, 0);
            
            if (ret == SOCKET_ERROR)
            {
            	int lastError=WSAGetLastError();
	int retCode = PROTO_ERR_SOCKET;
	if (lastError==WSAESHUTDOWN || lastError==WSAENETDOWN 
		|| lastError==WSAEHOSTUNREACH || lastError==WSAECONNRESET
		|| lastError==WSAENETRESET || lastError==WSAECONNABORTED)
	{
		retCode=PROTO_ERR_RESET_BY_PEER;
	}
				char buf[512] = {0};				
				sprintf(buf, "%s:%d Read recv error, m_socket: %d readLen: %d, ret: %d, WSAGetLastError():%d, return code:%d \n",__FILE__, __LINE__, (int)m_socket, readLen, ret, lastError, retCode);
				writeLog(ERROR_LEVEL, buf);
                return retCode;
            }

            if (ret == 0)
            {
				char buf[512] = {0};
				int retCode = PROTO_ERR_RESET_BY_PEER;
				sprintf(buf, "%s:%d Read recv error( disconnect by peer ),m_socket: %d, readLen: %d, ret: %d, WSAGetLastError():%d, return code:%d \n",__FILE__, __LINE__,(int)m_socket, readLen, ret, (int)WSAGetLastError(), retCode);
				writeLog(ERROR_LEVEL, buf);
                return PROTO_ERR_RESET_BY_PEER;
            }

            readLen += ret;
        }
        return 0;
    }

private:
    SOCKET  m_socket;
    bool    m_init;
};

int CSocketEx::count = 0;

#endif //__SOCKETEX__H
