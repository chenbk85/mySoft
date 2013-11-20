#ifndef __DATASTORAGE_UTILS_H__
#define __DATASTORAGE_UTILS_H__
//#include "config.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <uuid/uuid.h>
#include <string>
#include <time.h>
#include <vector>
#include <errno.h>
using namespace std;

typedef struct _GUID
{
        unsigned int Data1;
        unsigned short Data2;
        unsigned short Data3;
        unsigned char Data4[8];
} GUID, UUID;

static inline UUID CreateGuid()
{
        GUID guid;    
        uuid_generate(reinterpret_cast<unsigned char *>(&guid));
        return guid;
}

static inline string GuidToString(const GUID &guid)
{
        char buf[64] = {0};
        snprintf(   buf,
                        sizeof(buf),
                        "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                        guid.Data1, guid.Data2, guid.Data3, 
                        guid.Data4[0], guid.Data4[1],
                        guid.Data4[2], guid.Data4[3],
                        guid.Data4[4], guid.Data4[5],
                        guid.Data4[6], guid.Data4[7]);
        return std::string(buf);
}

static inline GUID StringToGuid(const string& strGUID)
{
        GUID oGUID = {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}};
        if ( strGUID.length() < 36 )
                return oGUID;
        int nStart = 0;
        if ( strGUID.substr(0, 1) == "{" )
                nStart = 1;
        oGUID.Data1 = strtoul(strGUID.substr(nStart, 8).c_str(), NULL, 16);
        nStart += 9;
        oGUID.Data2 = static_cast<unsigned short>(strtoul(strGUID.substr(nStart, 4).c_str(), NULL, 16));
        nStart += 5;
        oGUID.Data3 = static_cast<unsigned short>(strtoul(strGUID.substr(nStart, 4).c_str(), NULL, 16));
        nStart += 5;
        int nTmp = nStart + 1;
        for (int i=0; i<8; i++ )
        {
                if ( i < 2 )
                        nStart = nTmp - 1 +  2*i;
                else
                        nStart = nTmp + 2*i;
                oGUID.Data4[i] = static_cast<unsigned char>(strtoul(strGUID.substr(nStart, 2).c_str(), NULL, 16));
        }
        return oGUID;
}

static inline string GuidToString(const string& strGUID)
{
        GUID oGUID;
        memcpy(&oGUID, strGUID.c_str(), sizeof(GUID));
        return GuidToString(oGUID);
}

static inline string _getHashValue(const string& strKey, unsigned nSeed) 
{ 
        register unsigned nr=nSeed, nr2=nSeed + 3;
        const char* key = strKey.c_str();
        unsigned length = strKey.length();
        while (length--) 
        { 
                nr^= (((nr & 63)+nr2)*((unsigned) (unsigned char) *key++))+ (nr << 8); 
                nr2+=3; 
        }

        char szRet[128];
        //sprintf(szRet, _T("%x"), nr);
        sprintf(szRet, "%x", nr);
        string strRet(szRet);
        return strRet; 
}

static inline int CreateDir(const string& strDirectory)
{
        char DirName[256];   
        strcpy(DirName, strDirectory.c_str());   
        int i,len = strlen(DirName);

        if (DirName[len-1] != '/')
        {	
                strcat(DirName, "/");   
        }	      

        len = strlen(DirName);   

        for(i = 1; i < len; ++i)   
        {   
                if(DirName[i]=='/')   
                {   
                        DirName[i] = 0;   
                        if (access(DirName, 0) != 0)   
                        {   
                                if(mkdir(DirName, 0755)==-1)   
                                {   
                                        if (errno != EEXIST)
                                        { 
                                                perror("mkdir error");   
                                                return -1;
                                        }
                                }   
                        }   
                        DirName[i] = '/';   
                }  
        }	
        return 0;   
}

static inline void GetHashValue(const string& strIn, string& strOut)
{     
        strOut = _getHashValue(strIn,1) + _getHashValue(strIn,2) + _getHashValue(strIn,3);
}

static inline time_t _calcTime(int nMonth)
{
        time_t tNow;
        time(&tNow);

        struct tm* tm_Now = localtime(&tNow);

        tm_Now->tm_year -= nMonth / 12;

        if (tm_Now->tm_mon > nMonth)
        {
                tm_Now->tm_mon -= nMonth;
        }
        else
        {
                tm_Now->tm_year -= 1;
                tm_Now->tm_mon += (12 - nMonth);
        }
        return mktime(tm_Now);
}

static inline time_t tm2time(struct tm when)
{
        return timegm(&when);
}

class CTimeCounter
{
        public:
                CTimeCounter(){};
                virtual ~CTimeCounter(){};
                void Start()
                {
                        gettimeofday(&m_tvPre, &m_tz);
                }

                void End()
                {
                        gettimeofday(&m_tvAfter, &m_tz);
                }

                int GetTimeCount()
                {
                        return (m_tvAfter.tv_sec-m_tvPre.tv_sec)*1000+(m_tvAfter.tv_usec-m_tvPre.tv_usec)/1000;
                }

        private:
                struct timeval m_tvAfter;
                struct timeval m_tvPre;
                struct timezone m_tz;
};

static inline string ShowTime_Local(const time_t &time_)
{
        struct tm timeinfo;
        localtime_r(&time_, &timeinfo);

        char buffer[80];
        strftime (buffer, 80, "%Y-%m-%d %H:%M:%S", &timeinfo);
        return std::string(buffer);
}

static inline string ShowTime_UTC(const time_t &time_)
{
        struct tm timeinfo;
        gmtime_r(&time_, &timeinfo);

        char buffer[80];
        strftime (buffer, 80, "%Y-%m-%d %H:%M:%S", &timeinfo);
        return std::string(buffer);
}

class tic_timer
{
        public:
                tic_timer()
                {
                        gettimeofday(&start_, NULL);
                        temp_ = start_;
                }

                /*
                 * milliseconds version
                 */
                int64_t tic() // return milliseconds
                {
                        gettimeofday(&end_, NULL);
                        int64_t ms = (end_.tv_sec - temp_.tv_sec) * 1000 + (end_.tv_usec - temp_.tv_usec) / 1000;
                        temp_ = end_;
                        return ms;
                }

                int64_t reset() // return milliseconds
                {
                        int64_t ms = stop();
                        temp_ = start_ = end_;
                        return ms;
                }

                int64_t stop() // return milliseconds
                {
                        gettimeofday(&end_, NULL);
                        return (end_.tv_sec - start_.tv_sec) * 1000 + (end_.tv_usec - start_.tv_usec) / 1000;
                }

                /*
                 * microseconds version
                 */
                int64_t tic_micro() // return microseconds
                {
                        gettimeofday(&end_, NULL);
                        int64_t us = (end_.tv_sec - temp_.tv_sec) * 1000000 + (end_.tv_usec - temp_.tv_usec);
                        temp_ = end_;
                        return us;
                }

                int64_t reset_micro() // return microseconds
                {
                        int64_t us = stop_micro();
                        temp_ = start_ = end_;
                        return us;
                }

                int64_t stop_micro() // return microseconds
                {
                        gettimeofday(&end_, NULL);
                        return (end_.tv_sec - start_.tv_sec) * 1000000 + (end_.tv_usec - start_.tv_usec);
                }

        private:
                timeval start_;
                timeval temp_;
                timeval end_;
};

static inline std::string& RemoveSpace(std::string &str)
{
        str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());
        return str;
}

static inline std::string& Trim(std::string &s) 
{
        if (!s.empty()) {
                s.erase(0, s.find_first_not_of(" "));
                s.erase(s.find_last_not_of(" ") + 1);
        }
        return s;
}

static inline std::string TransformToLower(const std::string &input_str)
{
        std::string str = input_str;
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
}

static inline std::string TransformToUpper(const std::string &input_str)
{
        std::string str = input_str;
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
}


static inline void split_string(char *source, const char *spliter, std::vector<std::string> &container)
{
        char *token = NULL;
        char *saveptr = NULL;

        do {
                token = strtok_r(source, spliter, &saveptr);
                if (NULL == token) {
                        break;
                }
                container.push_back(token);
                source = NULL;
        } while (1);
}

static inline void SplitString(const std::string &source, const char *spliter, std::vector<std::string> &container)
{
        std::string bak = source.c_str();
        split_string(const_cast<char*>(bak.c_str()), spliter, container);
}


/*
static inline string TransformToHigher(const string& str)
{
        string retStr;
        retStr.reserve(str.length());
        string::const_iterator itr = str.begin();

        for ( ;itr != str.end(); ++itr )
        {
                if ( *itr >= 'a' && *itr <= 'z' )
                {
                        char ch = *itr - 'a' + 'A' ;
                        retStr.push_back(ch);
                }
                else
                {
                        retStr.push_back(*itr);
                }
        }

        return retStr;
}


static inline string TransformToLower(const string& str)
{
        string retStr;
        retStr.reserve(str.length());
        string::const_iterator itr = str.begin();

        for ( ;itr != str.end(); ++itr )
        {
                if ( *itr >= 'A' && *itr <= 'Z' )
                {
                        char ch = *itr - 'A' + 'a';
                        retStr.push_back(ch);
                }
                else
                {
                        retStr.push_back(*itr);
                }
        }

        return retStr;
}
*/


#endif

