#include <errno.h>

#include "schedule.h"


schedule::sched_map_t schedule::scheds_map;
ScheduleFactory::sched_factory_map_t ScheduleFactory::sched_factories_map;

static int sched_rsp(MessageResponse* rsp, void *p)
{
	schedule* sched = static_cast<schedule *>(p);
	return sched->response(rsp);
}

schedule::schedule(processor* proc)
	: proc_(proc), status_(0)
{
	proc_->set_async_fun(sched_rsp, this);
}

int schedule::_register(const std::string& name, schedule* sched)
{
	sched_map_t::iterator iter = scheds_map.find(name);
	if (iter == scheds_map.end()) {
		scheds_map.insert(std::make_pair(name, sched));
	}
	else {
		errno = EEXIST;
		return -1;
	}

	return 0;
}

void schedule::unregister(const std::string& name)
{
	sched_map_t::iterator iter = scheds_map.find(name);
	if (iter != scheds_map.end()) {
		// TODO: check reference
		scheds_map.erase(iter);
	}
	else {
		errno = EEXIST;
	}

	return;
}

schedule* schedule::get(const std::string& name)
{
	sched_map_t::iterator iter = scheds_map.find(name);
	if (iter != scheds_map.end()) {	
		return iter->second;
	}

	return NULL;
}

int ScheduleFactory::_register(const std::string& name, ScheduleFactory* sf)
{
	sched_factory_map_t::iterator iter = sched_factories_map.find(name);
	if (iter == sched_factories_map.end()) {
		sched_factories_map.insert(std::make_pair(name, sf));
	}
	else {
		errno = EEXIST;
		return -1;
	}

	return 0;
}

void ScheduleFactory::unregister(const std::string& name)
{
	sched_factory_map_t::iterator iter = sched_factories_map.find(name);
	if (iter != sched_factories_map.end()) {
		// TODO: check ref
		sched_factories_map.erase(iter);
	}
	else {
		errno = EEXIST;
		return;
	}

	return;
}

ScheduleFactory* ScheduleFactory::get(const std::string& name)
{
	sched_factory_map_t::iterator iter = sched_factories_map.find(name);
	if (iter != sched_factories_map.end()) {
		return iter->second;
	}

	return NULL;
}

