/* schedule.h
 *
**/
#ifndef __SCHEDULE__H
#define __SCHEDULE__H

#include <map>
#include "processor.h"
#include "message_block.h"

class schedule {
public:
	schedule(processor* proc);
	virtual ~schedule() { /* nothing */ }
public:
	virtual int on_request(MessageRequest* req) = 0;
	virtual int response(MessageResponse* rsp) = 0;
public:
	processor* proc() const { return proc_; }
	void proc(processor* proc) { proc_ = proc; }

	int status(int status) { return status_ = status; }
	int status() const { return status_; }

	bool is_stopped() const { return status_ != 0; }
public:
	static int _register(const std::string& name, schedule* sched);
	static void unregister(const std::string& name);
	static schedule* get(const std::string& name);
private:
	processor* proc_;
	volatile int status_;

	typedef std::map<std::string, schedule*> sched_map_t; 
	static sched_map_t scheds_map;
};

class ScheduleFactory {
public:
	virtual ~ScheduleFactory() {}
public:
	virtual schedule* create(processor* proc, const std::map<std::string, std::string>& params) = 0;
	virtual void destroy(schedule* sched) = 0;
public:
	static int _register(const std::string& name, ScheduleFactory* sf);
	static void unregister(const std::string& name);
	static ScheduleFactory* get(const std::string& name);
private:
	typedef std::map<std::string, ScheduleFactory*> sched_factory_map_t;
	static sched_factory_map_t sched_factories_map;
};

#endif /*! __SCHEDULE__H */

