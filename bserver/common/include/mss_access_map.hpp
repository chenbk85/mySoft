#ifndef _MSS_ACCESS_HPP__
#define _MSS_ACCESS_HPP__

#include <algorithm>
#include <map>
#include <stdarg.h>
#include "thread_lock.h"
#include "load_balance.hpp"
#include "beyondy/timedout_countdown.hpp"
#include "mstar_definition.h"

static long over_flow_min_time_ = 3000; //the minimum over flow sleep time (ms)
static long over_flow_max_time_ = 5000; //the maximal over flow sleep time (ms)
static bool has_seed = false;

static bool setOverFlowTimeMS(long min_time, long max_time)
{
        if(min_time < 1000 || min_time > max_time ) return false;

        over_flow_min_time_ = min_time;
        over_flow_max_time_ = max_time;

        return true;
}

static long rand_num()
{
        long max = over_flow_max_time_;
        long min = over_flow_min_time_;

        if(!has_seed)
        {
                srand(time(NULL));
                has_seed = true;
        }

        if (max <= min) max = min + 1;
        return (min + rand() % (max - min));
}

#ifdef WIN32
extern bool write_log_(const char *log, int log_len);
#endif

template <typename T_MAP, typename T_REQ, typename T_RES>
static int safe_request(T_MAP& mapProto, const char* key, unsigned int cmd, const T_REQ& req, int req_size, T_RES& res, long timedout, int retries = 3, int stop_retcode_1 = 0, ...)
{
        //beyondy::TimedoutCountdown countdown(timedout);
        std::vector<int> stop_returns;
        int retval = -1;
        va_list ap;

        stop_returns.push_back(stop_retcode_1);
        //modified by jackie 2010-02-24
        if (stop_retcode_1 != 0)
        {
                va_start(ap, stop_retcode_1);
                do {
                        stop_retcode_1 = va_arg(ap, int);
                        stop_returns.push_back(stop_retcode_1);
                } while (stop_retcode_1 != 0);
        }

        char logbuf[1000];

        //modified end
        int i = 0;
        int retry_times = 0;
        //int over_flow_cnt = 0;
        for (i = 0; i < retries; /* sometimes need retry unconditionally */)
        {
                typename T_MAP::ProtoType* proto = mapProto.get_object(key);
                if (!proto)
		{
#ifdef WIN32
                        int result = sprintf(logbuf, "%s:%d, %s: Failed to get object for cmd %u.\r\n",
                                        __FILE__, __LINE__, __FUNCTION__, cmd);
                        write_log_(logbuf, result);
#endif

                        retval = -1;
                        break;
                }

                beyondy::TimedoutCountdown countdown(timedout);

                retval = proto->Request(cmd, req, req_size, res, countdown.Update());
#ifdef WIN32
                int result = sprintf(logbuf, "%s:%d, %s: proto->Request returns code %d for cmd %u.\r\n",
                                __FILE__, __LINE__, __FUNCTION__, retval, cmd);
                write_log_(logbuf, result);
#endif
                

                if (retval == PROTO_ERR_DISCONNECT_BY_PEER && retry_times < 3)
                {
                        // retry unconditionally
                        retry_times ++;
                        mapProto.put_object(key, proto, 1);
#ifdef WIN32
                        result = sprintf(logbuf, "%s:%d, %s: Disconnected by peer for cmd %u.\r\n",
                                        __FILE__, __LINE__, __FUNCTION__, cmd);
                        write_log_(logbuf, result);
#endif
                        

                        continue;
                }
                else if (retval == PROTO_ERR_ENCODE)
                {
#ifdef WIN32
                        result = sprintf(logbuf, "%s:%d, %s: Failed to encode request for cmd %u.\r\n",
                                        __FILE__, __LINE__, __FUNCTION__, cmd);
                        write_log_(logbuf, result);
#endif
                        
                        mapProto.put_object(key, proto, 0);
                        break;	// do not need retry
                }
                else if (retval != 0)
                {
                        mapProto.put_object(key, proto, retval);

                        // re-try for network error
                }
                else
                {
                        // check return code,
                        std::vector<int>::iterator itr = std::find(stop_returns.begin(),stop_returns.end(), res.retCode);
                        if ( itr != stop_returns.end() )
                        {
                                // return OK, no need re-try
                                mapProto.put_object(key, proto, 0);
                                break;
                        }

                        mapProto.put_object(key, proto, 1);
                        // re-try

                        if(res.retCode == E_OVER_FLOW)
                        {
                                long wait_time_ = rand_num();
#ifdef WIN32
                                Sleep(wait_time_);
#else
                                usleep(wait_time_*1000);
#endif
                                countdown.Restart(); 

                                --i;

                        }

                }

                ++i;
        }

        // modified by jackie 2010-02-24
        //if (i == retries) {
        //	return PROTO_ERR_SERVER_BUSY;
        //}

        // modified end
        return retval;
}

        template <typename T_MAP, typename T_REQ, typename T_RES>
static int safe_request_ex(T_MAP& mapProto, const char* key,char*& ip,unsigned int& iplen, unsigned int cmd, const T_REQ& req, int req_size, T_RES& res, long timedout, int retries = 3,int stop_retcode_1 = 0, ...)
{
        //beyondy::TimedoutCountdown countdown(timedout);
        std::vector<int> stop_returns;
        int retval = -1;
        va_list ap;

        stop_returns.push_back(stop_retcode_1);
        //modified by jackie 2010-02-24
        if (stop_retcode_1 != 0)
        {
                va_start(ap, stop_retcode_1);
                do {
                        stop_retcode_1 = va_arg(ap, int);
                        stop_returns.push_back(stop_retcode_1);
                } while (stop_retcode_1 != 0);
        }

        //modified end
        int i = 0;
        int retry_times = 0;
        visit_timeout = 60000;
        for (i = 0; i < retries; /* sometimes need retry unconditionally */)
        {
                typename T_MAP::ProtoType* proto = mapProto.get_object(key,ip);
                if (proto==NULL)
                {
                        return retval;
                }

                beyondy::TimedoutCountdown countdown(timedout);

                retval = proto->Request(cmd, req, req_size, res, countdown.Update());
                if (retval == PROTO_ERR_DISCONNECT_BY_PEER && retry_times < 3)
                {
                        // retry unconditionally
                        retry_times ++;
                        mapProto.put_object(key, proto, 1);
                        continue;
                }
                else if (retval == PROTO_ERR_ENCODE)
                {
                        mapProto.put_object(key, proto, 0);
                        break;	// do not need retry
                }
                else if (retval != 0)
                {
                        mapProto.put_object(key,proto, 1);
                        // re-try for network error
                }
                else
                {
                        // check return code,
                        std::vector<int>::iterator itr = std::find(stop_returns.begin(),stop_returns.end(), res.retCode);
                        if ( itr != stop_returns.end() )
                        {
                                // return OK, no need re-try
                                mapProto.put_object(key,proto, 0);
                                ip = proto->GetIp();
                                iplen = strlen(ip);
                                break;
                        }

                        mapProto.put_object(key, proto, 1);
                        // re-try
                }

                ++i;

                if(i < retries)
                {
#if defined WIN32
                        Sleep(i*2000);
#else
                        sleep(i*2);
#endif
                }
        }

        // modified end
        return retval;
}

        template <typename T_MAP, typename T_REQ, typename T_RES>
static int safe_request_ex(T_MAP& mapProto, const char* key, std::string& ip, unsigned int cmd, const T_REQ& req, int req_size, T_RES& res, long timedout, int retries = 3,int stop_retcode_1 = 0, ...)
{
        //beyondy::TimedoutCountdown countdown(timedout);
        std::vector<int> stop_returns;
        int retval = -1;
        va_list ap;

        stop_returns.push_back(stop_retcode_1);
        //modified by jackie 2010-02-24
        if (stop_retcode_1 != 0)
        {
                va_start(ap, stop_retcode_1);
                do {
                        stop_retcode_1 = va_arg(ap, int);
                        stop_returns.push_back(stop_retcode_1);
                } while (stop_retcode_1 != 0);
        }

        //modified end
        int i = 0;
        int retry_times = 0;
        visit_timeout = 60000;
        for (i = 0; i < retries; /* sometimes need retry unconditionally */)
        {
                typename T_MAP::ProtoType* proto = mapProto.get_object(key, ip.empty() ? 0 : ip.c_str());
                if (proto==NULL)
                {
                        return retval;
                }

                beyondy::TimedoutCountdown countdown(timedout);

                retval = proto->Request(cmd, req, req_size, res, countdown.Update());
                if (retval == PROTO_ERR_DISCONNECT_BY_PEER && retry_times < 3)
                {
                        // retry unconditionally
                        retry_times ++;
                        mapProto.put_object(key, proto, 1);
                        continue;
                }
                else if (retval == PROTO_ERR_ENCODE)
                {
                        mapProto.put_object(key,proto, 0);
                        break;	// do not need retry
                }
                else if (retval != 0)
                {
                        mapProto.put_object(key,proto, 1);
                        // re-try for network error
                }
                else
                {
                        // check return code,
                        std::vector<int>::iterator itr = std::find(stop_returns.begin(),stop_returns.end(), res.retCode);
                        if ( itr != stop_returns.end() )
                        {
                                // return OK, no need re-try
                                mapProto.put_object(key,proto, 0);
                                char* buf = proto->GetIp();
                                ip = buf;
                                free(buf);
                                break;
                        }

                        mapProto.put_object(key, proto, 1);
                        // re-try
                }

                ++i;

                if(i < retries)
                {
#if defined WIN32
                        Sleep(i*2000);
#else
                        sleep(i*2);
#endif
                }
        }

        // modified end
        return retval;
}

template <typename T>
class mss_msg
{
        public:
                mss_msg(){
                }
                ~mss_msg(){
                }
        public:
                RouteManager<int> rm_obj;
                std::multimap<std::string, T* > mm_obj; //addr - obj
};

template <typename T>
class mss_access_map
{
        public:
                typedef T ProtoType;
                mss_access_map();
                ~mss_access_map();

        protected:
                void clean_multimap();

        public:

                int release_connection(const std::string& key);

                int add_addr_weight(const std::string& key, const std::string& addr, const int weight);

                T* new_object(const std::string& addr);

                T* get_object(const std::string& key);
                T* get_object(const std::string& key,const char* ip);

                void put_object(const std::string& addr, T* proto, const int retval);

                void put_route(const std::string& addr, T* proto, const int retval);

        protected:
                thread_lock lock_;

                typedef RouteInfo<int> ROUTE_INFO;
                std::map<std::string, mss_msg<T> > mm_obj_; //key - object
                std::map<T*, ROUTE_INFO*> map_T_route_;
};

template <typename T> mss_access_map<T>::mss_access_map()
{
        map_T_route_.clear();
}

template <typename T> mss_access_map<T>::~mss_access_map()
{
        clean_multimap();
}

template <typename T> int mss_access_map<T>::add_addr_weight(const std::string& key, const std::string& addr, const int weight)
{
        auto_lock lock(lock_);
        ROUTE_INFO* ri = mm_obj_[key].rm_obj.addRoute(addr, weight);
        if (ri == NULL )
        {
                return -1;
        }
        return 0;
}

template <typename T> T* mss_access_map<T>::new_object(const std::string& addr)
{
        T* obj = new T((char*)addr.c_str());

        return obj;
}

template <typename T> T* mss_access_map<T>::get_object(const std::string& key)
{
        auto_lock lock(lock_);

        ROUTE_INFO* route_info = mm_obj_[key].rm_obj.getRoute();
        if(route_info == NULL)
        {
                return NULL;
        }

        typename std::multimap<std::string, T* >::iterator itr = mm_obj_[key].mm_obj.find(route_info->ip());

        if ( itr == mm_obj_[key].mm_obj.end() )
        {
                T* obj = new_object(route_info->ip());
                map_T_route_[obj] = route_info;
                return obj;
        }

        T* obj = itr->second;
        map_T_route_[obj] = route_info;
        mm_obj_[key].mm_obj.erase(itr);

        return obj;
}

template <typename T> T* mss_access_map<T>::get_object(const std::string& key,const char* ip)
{
        auto_lock lock(lock_);
        ROUTE_INFO* route_info;
        if (ip == NULL)
        {
                route_info = mm_obj_[key].rm_obj.getRoute();
        }
        else
        {
                route_info = mm_obj_[key].rm_obj.getRoute(ip);
        }

        if(route_info == NULL)
        {
                return NULL;
        }

        typename std::multimap<std::string, T* >::iterator itr = mm_obj_[key].mm_obj.find(route_info->ip());

        if ( itr == mm_obj_[key].mm_obj.end() )
        {
                T* obj = new_object(route_info->ip());
                map_T_route_[obj] = route_info;
                return obj;
        }

        T* obj = itr->second;
        map_T_route_[obj] = route_info;
        mm_obj_[key].mm_obj.erase(itr);

        return obj;
}

template <typename T> void mss_access_map<T>::put_object(const std::string& key, T* obj, const int retval)
{
        auto_lock lock(lock_);
        if ( obj == NULL )
                return ;

        mm_obj_[key].mm_obj.insert(std::make_pair(map_T_route_[obj]->ip(), obj));

        if ( retval == 0 )
                mm_obj_[key].rm_obj.putRoute(map_T_route_[obj], STAT_OK);
        else if (PROTO_ERR_CONNECT == retval)
                mm_obj_[key].rm_obj.putRoute(map_T_route_[obj], STAT_UNAVAILABLE);
        else
                mm_obj_[key].rm_obj.putRoute(map_T_route_[obj], STAT_NOT_OK);

        map_T_route_.erase(obj);
}

template <typename T> void mss_access_map<T>::put_route(const std::string& key, T* obj, const int retval)
{
        auto_lock lock(lock_);

        if ( retval == 0 )
                mm_obj_[key].rm_obj.putRoute(map_T_route_[obj], STAT_OK);
        else if (PROTO_ERR_CONNECT == retval)
                mm_obj_[key].rm_obj.putRoute(map_T_route_[obj], STAT_UNAVAILABLE);
        else
                mm_obj_[key].rm_obj.putRoute(map_T_route_[obj], STAT_NOT_OK);

        map_T_route_.erase(obj);
}

template <typename T> void mss_access_map<T>::clean_multimap()
{
        auto_lock lock(lock_);
        typename std::map<std::string, mss_msg<T> >::iterator itr = mm_obj_.begin();
        for ( ; itr != mm_obj_.end(); ++itr )
        {
                typename std::multimap<std::string, T* >::iterator itr_obj = itr->second.mm_obj.begin();
                for ( ; itr_obj != itr->second.mm_obj.end(); ++ itr_obj )
                {
                        delete itr_obj->second;
                }
                itr->second.mm_obj.clear();
        }
        mm_obj_.clear();
}

template <typename T> int mss_access_map<T>::release_connection(const std::string& key)
{
        auto_lock lock(lock_);
        typename std::map<std::string, mss_msg<T> >::iterator itr = mm_obj_.find(key);

        if ( itr != mm_obj_.end() )
        {
                typename std::multimap<std::string, T* >::iterator itr_obj = itr->second.mm_obj.begin();
                for ( ; itr_obj != itr->second.mm_obj.end(); ++ itr_obj )
                {
                        delete itr_obj->second;
                }
                itr->second.mm_obj.clear(); 
	mm_obj_.erase(itr);
        }

        return 0;
}

#endif /*_MSS_ACCESS_HPP__*/
