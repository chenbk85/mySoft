#ifndef _LOAD_BALANCE__H
#define _LOAD_BALANCE__H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <time.h>

#define STAT_OK   0
#define STAT_UNAVAILABLE   1
#define STAT_NOT_OK -1
#define W_MIN   1
using namespace std;




template <typename T>
struct RouteInfo {
public:
	void setData(T* data){data_ = data;}
	void set_ip(string ip){ip_ = ip;}
	void set_conf_weight(long conf_weight){conf_weight_ = conf_weight;}
	void set_real_weight(long real_weight){real_weight_ = real_weight;}

	T* data() const { return data_; }
	string& ip()  { return ip_; }
	long conf_weight() const { return conf_weight_; }
	long real_weight() const { return real_weight_; }
private:
	string ip_;
	long conf_weight_;
	long real_weight_;
	T* data_;

};

template <typename T>
class RouteManager
{
public:
	RouteManager ();
	RouteManager (long alpha_inc, long alpha_dec);
	~RouteManager ()
	{
		typename vector<struct RouteInfo<T> >::iterator theIter;
		for(theIter = vRoute_.begin(); theIter != vRoute_.end();theIter++)
		{
			if(theIter->data() != NULL)
			{
				delete theIter->data();
				theIter->setData(NULL);
			}
		}
		vRoute_.clear();
	};

	RouteInfo<T>* getRoute()
	{
		if (vRoute_.empty()) {
			return NULL;
		}

		int gcd = getGCD();
		int max_weight = getMaxWeight();
		while (true) {
			last_attemper_ = (last_attemper_ + 1) % vRoute_.size();
			if (last_attemper_ == 0)
			{
				current_attemper_weight_ = current_attemper_weight_ - gcd;
				if (current_attemper_weight_ <= 0)
				{
					current_attemper_weight_ = max_weight;
					if (current_attemper_weight_ == 0)
						return NULL;
				}
			}

			if (vRoute_[last_attemper_].real_weight() >= current_attemper_weight_)
			{
				return &vRoute_[last_attemper_];
			}
		}
	}

	vector<RouteInfo<int>* > getAllRoute()
	{
		vector<RouteInfo<int>* > routeInfo;
		for (size_t i = 0; i < vRoute_.size(); ++i)
		{
			routeInfo.push_back(&vRoute_[i]);
		}
		return routeInfo;
	}

	RouteInfo<T>* getRoute(const std::string& ip);

	void putRoute(struct RouteInfo<T>* ri, int status);
	RouteInfo<T>* addRoute(const string& ip, long weight);
	int delRoute(const string& ip);

	int setAlphaInc(long alpha_inc_);
	int setAlphaDec(long alpha_dec);
	int getSumWeight(){return sum_weight_;}
private:
	int GCD(int a, int b);
	int NGCD(const std::vector<int>& a, size_t n);
	int getGCD(void)
	{
		std::vector<int> vec_gcd;
		for (typename vector<struct RouteInfo<T> >::iterator iter = vRoute_.begin();
				iter != vRoute_.end();
				++iter)
		{
			vec_gcd.push_back(iter->real_weight());
		}

		return NGCD(vec_gcd, vec_gcd.size());
	}
	int getMaxWeight(void)
	{
		int max_weight = 0;
		for (typename vector<struct RouteInfo<T> >::iterator iter = vRoute_.begin();
				iter != vRoute_.end();
				++iter)
		{
			if ( max_weight < iter->real_weight() )
				max_weight = iter->real_weight();
		}
		return max_weight;
	}

private:
	int last_attemper_;
	int current_attemper_weight_;

private:
	vector<struct RouteInfo<T> > vRoute_;
	long alpha_inc_;
	long alpha_dec_;
	long sum_weight_;
};

template <typename T>
RouteManager<T>::RouteManager ():
			last_attemper_(0),
			current_attemper_weight_(0)
{
	sum_weight_ = 0;
	this->alpha_inc_ = 91;
	this->alpha_dec_ = 95;
	srand(time(0));
}

template <typename T>
RouteManager<T>::RouteManager (long alpha_inc, long alpha_dec):
					last_attemper_(0),
					current_attemper_weight_(0)
{
	sum_weight_ = 0;
	this->alpha_inc_ = alpha_inc;
	this->alpha_dec_ = alpha_dec;
	srand(time(0));
}

template <typename T>int  RouteManager<T>::setAlphaInc (long alpha_inc)
{
	if (alpha_inc > 0 && alpha_inc <= 1000)
	{
		this->alpha_inc_ = alpha_inc;
		return 0;
	}

	return -1;
}

template <typename T>int  RouteManager<T>::setAlphaDec(long alpha_dec)
{
	if (alpha_dec > 0 && alpha_dec <= 1000)
	{
		this->alpha_dec_ = alpha_dec;
		return 0;
	}

	return -1;
}

template <typename T>int  RouteManager<T>::GCD(int a, int b)
{
    if (a < b)
    {
		a = a ^ b;
		b = a ^ b;
		a = a ^ b;
    }
    if (b == 0)
        return a;
    else
        return GCD(b, a%b);
}

template <typename T>int  RouteManager<T>::NGCD(const std::vector<int>& a, size_t n)
{
	if ( n == 1 )
		return a[0];

	return GCD(a[n-1], NGCD(a, n-1));
}


//template <typename T>
//RouteInfo<T>* RounteManager<T>::getRoute()
//{
//	while (true) {
//		if (last_attemper_ == 0)
//		{
//			current_attemper_weight_ = current_attemper_weight_ - getGCD();
//			if (current_attemper_weight_ <= 0)
//			{
//				current_attemper_weight_ = getMaxWeight();
//				if (current_attemper_weight_ == 0)
//					return NULL;
//			}
//		}
//		else
//		{
//			last_attemper_ = (last_attemper_ + 1) % vRoute_.size();
//		}
//		if (vRoute_[last_attemper_].real_weight() >= current_attemper_weight_)
//			return &vRoute_[last_attemper_];
//	}
//}

//template <typename T>
//RouteInfo<T>* RouteManager<T>::getRoute(vector<struct RouteInfo<T> *> &vRouteUsed)
//{
//	int rand_weight;
//	int sum_weight = sum_weight_;
//	for (typename vector<struct RouteInfo<T> *>::size_type i = 0; i < vRouteUsed.size(); i++ )
//	{
//		sum_weight -= vRouteUsed[i]->real_weight();
//	}
//
//	if (0 == sum_weight)
//	{
//		sum_weight = sum_weight_;
//		vRouteUsed.clear();
//	}
//
//	rand_weight = rand()%sum_weight;
//
//#ifndef WIN32
//	syslog()->debug("sum_weight = %d,rand = %d.", sum_weight, rand_weight);
//#endif
//	for (typename vector<struct RouteInfo<T> >::iterator iter = vRoute_.begin();
//		iter != vRoute_.end();
//			++iter)
//	{
//		if (std::find(vRouteUsed.begin(), vRouteUsed.end(), &*iter) != vRouteUsed.end())
//			continue;
//
//		rand_weight -= iter->real_weight();
//		if (rand_weight  < 0)
//		{
//			vRouteUsed.push_back(&*iter);
//			return &*iter;
//		}
//	}
//
//
//	return NULL;
//}

template <typename T> RouteInfo<T>* RouteManager<T>::getRoute(const string& ip)
{
	for (typename vector<struct RouteInfo<T> >::size_type i = 0; i < vRoute_.size(); i++ )
	{
		if (vRoute_[i].ip() == ip)
		{
			return &vRoute_[i];
		}
	}

	return NULL;
}

template <typename T>void RouteManager<T>::putRoute(struct RouteInfo<T>* ri, int status)
{
	long old_weight = ri->real_weight();
    if (status == STAT_UNAVAILABLE)
    {
        ri->set_real_weight(W_MIN);
    }
	else if (status == STAT_OK)
	{
		long beta = 105 - alpha_inc_;
		ri->set_real_weight((ri->real_weight() * alpha_inc_ + ri->conf_weight() * beta)/100) ;
		if (ri->real_weight() > ri->conf_weight())
		{
			ri->set_real_weight(ri->conf_weight());
		}
	}
	else
	{
		long beta = 105 - alpha_dec_;
		ri->set_real_weight((ri->real_weight() * alpha_dec_ - ri->conf_weight() * beta)/100) ;
		if (ri->real_weight() < W_MIN)
		{
			ri->set_real_weight(W_MIN) ;
		}
	}

	sum_weight_ +=  (ri->real_weight() - old_weight);
}

template <typename T> RouteInfo<T>* RouteManager<T>::addRoute(const string& ip,  long weight)
{
	struct RouteInfo<T> routeInfo;
	routeInfo.set_ip(ip);

	if (weight <= 0)
	{
		return NULL;
	}
	routeInfo.set_conf_weight(weight);
	routeInfo.set_real_weight(weight);
	T *data = new T;
	if (data == NULL)
	{
		return NULL;
	}
	routeInfo.setData(data) ;
	for (typename vector<struct RouteInfo<T> >::size_type i = 0; i < vRoute_.size(); i++ )
	{
		if (vRoute_[i].ip()  == ip)
		{
			return NULL;
		}
	}

	vRoute_.push_back(routeInfo);
	sum_weight_ += weight;

	return &vRoute_[vRoute_.size() - 1];
}

template <typename T> int RouteManager<T>::delRoute(const string& ip)
{
	typename vector<struct RouteInfo<T> >::iterator ite = vRoute_.begin();
	for (; ite != vRoute_.end(); )
	{
		if (ite->ip() == ip)
		{
			sum_weight_ -= ite->real_weight();
			if(ite->data() != NULL)
			{
				delete ite->data();
				ite->setData(NULL);
			}
			vRoute_.erase(ite);
			return 0;
		}
	}

	return -1;
}

#endif //_LOAD_BALANCE__H

