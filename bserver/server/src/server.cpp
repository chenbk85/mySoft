#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <exception>
#include <numeric>
#include <limits>
#include <csignal>
#include <cerrno>
#include <pthread.h>
#include <dlfcn.h>

#include "xmlconfig.h"
#include "acceptor.h"
#include "schedule.h"
#include "protocol.h"
#include "processor.h"
#include "log.h"

// registration
#include "proto_h16.h"
#include "sched_mt.h"

struct listener_config {
	std::string addr_;
	std::string proto_;
	std::string proc_;
};

struct processor_config {
	std::string name_;
	std::string sched_;
	std::string so_path_;
	std::map<std::string, std::string> params_;
};

struct server_config {
	std::string name_;
	std::string log_properties_;
	std::string log_level_;
	std::vector<struct listener_config> listeners_;
	std::vector<struct processor_config> processors_;
};

static int load_config(const char* file, struct server_config& sc)
{
	try {
		xmlconfig config(file);
		xmlconfig::icontext ctx = config.get_context("/global");

		sc.name_ = ctx.get_string("name", "server");
		
		ctx = config.get_context("/log");
		sc.log_properties_ = ctx.get_string("property-file", "../etc/log.properties");
		sc.log_level_ = ctx.get_string("level", "ERROR");

		for (ctx = config.get_context("/listener-list/listener");
			ctx != xmlconfig::end;
				++ctx) {
			struct listener_config lc;
			lc.addr_ = ctx.get_string("address", "");
			lc.proto_ = ctx.get_string("protocol", "H16/1.0");
			lc.proc_ = ctx.get_string("processor", "");

			if (lc.addr_.empty() || lc.proto_.empty() || lc.proc_.empty()) {
				fprintf(stderr, "listener(addr: %s, proto: %s, processor: %s) does not set well.\n",
					lc.addr_.c_str(), lc.proto_.c_str(), lc.proc_.c_str());
				return -1;
			}

			sc.listeners_.push_back(lc);
		}

		for (ctx = config.get_context("/processor-list/processor");
			ctx != xmlconfig::end;
				++ctx) {
			struct processor_config pc;
			pc.name_ = ctx.get_string("name", "");
			pc.sched_ = ctx.get_string("schedule", "");
			pc.so_path_ = ctx.get_string("file", "");
			for (xmlconfig::icontext sub = ctx.get_context("p");
				sub != xmlconfig::end;
					++sub) {
				std::string n = sub.get_string("i");
				std::string v = sub.get_string("v");
				pc.params_[n] = v;
			}

			if (pc.name_.empty() || pc.sched_.empty() || pc.so_path_.empty()) {
				fprintf(stderr, "processor(name: %s, schedule: %s, file: %s) does not configure well.\n",
					pc.name_.c_str(), pc.sched_.c_str(), pc.so_path_.c_str());
				return -1;
			}

			sc.processors_.push_back(pc);
		}
	}
	catch (std::exception& ex)
	{
		fprintf(stderr, "parse %s failed: %s\n", file, ex.what());
		return -1;
	}
	catch (...) {
		return -1;
	}

	return 0;
}

static void dump_config(struct server_config& sc)
{
	fprintf(stdout, "log-property-file: %s\n", sc.log_properties_.c_str());
	fprintf(stdout, "log-level: %s\n", sc.log_level_.c_str());
}

static processor* load_processor(const char* file)
{
	void *dh = dlopen(file, RTLD_NOW);
	if (dh == NULL) {
		SYSLOG_ERROR("dlopen(%s) failed: %s", file, dlerror());
		return NULL;
	}

	processor* (*fcreate)();
	processor* inst;

	if ((fcreate = (processor* (*)())dlsym(dh, "create")) == NULL) {
		char *err = dlerror();
		SYSLOG_ERROR("no create find in %s: %s.", file, err ? err : "no create found");
		goto close_dh;
	}

	inst = (*fcreate)();
	if (inst == NULL) {
		SYSLOG_ERROR("%s create an instance failed: %m", file);
		goto close_dh;
	}

	return inst;
close_dh:
	dlclose(dh);
	return NULL;
}

static int init_processor(const struct processor_config& pc, EventManager* emgr)
{
	processor* proc;
	ScheduleFactory* sf;
	schedule* sched;

	if ((proc = load_processor(pc.so_path_.c_str())) == NULL) {
		SYSLOG_ERROR("load processor from file %s failed.", pc.so_path_.c_str());
		return -1;
	}

	if ((sf = ScheduleFactory::get(pc.sched_)) == NULL) {
		SYSLOG_ERROR("unknown schedule factory: %s", pc.sched_.c_str());
		goto fini_proc;
	}

	if ((sched = sf->create(proc, pc.params_)) == NULL) {
		SYSLOG_ERROR("create schedule for %s in %s mode failed: %m", pc.name_.c_str(), pc.sched_.c_str());
		goto fini_proc;
	}

	if (schedule::_register(pc.name_, sched) < 0) {
		SYSLOG_ERROR("can not register processor: (%s %s): %m", pc.name_.c_str(), pc.sched_.c_str());
		goto destroy_sched;
	}

	return 0;
destroy_sched:
	sf->destroy(sched);
fini_proc:
	proc->fini();
	return -1;
}

static int init_processor_list(const struct server_config& sc, EventManager* emgr)
{
	for (std::vector<struct processor_config>::const_iterator iter = sc.processors_.begin();
		iter != sc.processors_.end();
			++iter) {
		if (init_processor(*iter, emgr) < 0) {
			SYSLOG_ERROR("init processor(name: %s, schedule: %s, file: %s) failed.",
				iter->name_.c_str(),
				iter->sched_.c_str(),
				iter->so_path_.c_str());
			return -1;
		}
	}

	return 0;
}

static int init_listener(const struct listener_config& lc, EventManager* emgr)
{
	protocol* proto = protocol::get(lc.proto_.c_str());
	if (proto == NULL) {
		SYSLOG_ERROR("can not find protocol: %s for listener: %s", lc.addr_.c_str(), lc.proto_.c_str());
		return -1;
	}

	schedule* sched = schedule::get(lc.proc_);
	if (sched == NULL) {
		SYSLOG_ERROR("can not find processor: %s for listener: %s", lc.addr_.c_str(), lc.proc_.c_str());
		return -1;
	}

	// TODO: handle UDP specically
	try {
		Acceptor* acceptor = new Acceptor(lc.addr_.c_str(), proto, sched, emgr);
		if (acceptor != NULL) {
			emgr->add_poller(acceptor, EPOLLIN);
		}
		else {
			assert(0);
		}
	}
	catch (std::exception& ex) {
		SYSLOG_ERROR("new acceptor at %s failed: %s.", lc.addr_.c_str(), ex.what());
		return -1;
	}

	return 0;
}

static int init_listener_list(const struct server_config& sc, EventManager* emgr)
{
	for (std::vector<struct listener_config>::const_iterator iter = sc.listeners_.begin();
		iter != sc.listeners_.end();
			++iter) {
		if (init_listener(*iter, emgr) < 0) {
			SYSLOG_ERROR("init listener(address: %s, protocol: %s, processor: %s) failed: %m",
					iter->addr_.c_str(),
					iter->proto_.c_str(),
					iter->proc_.c_str());
			return -1;
		}
	}

	return 0;
}

static void usage(const char* p)
{
	fprintf(stdout, "Usage: %s [options]\n"
			" -h | --help            show this hellp message.\n"
			" -v | --version         show version.\n"
			" -V                     show version, including processors.\n"
			" -c config-file         specify configuration file. default is ../etc/server.xml.\n"
			" -l seconds             run specified seconds and exit.\n"		 
			" -d                     run non-daemon mode.\n",
			p);
	exit(0);
}

int main(int argc, char **argv)
{
	// TODO:
	proto_h16::_register();
	schedule_mt::_register();

	// parse parameters
	const char* config_file = "../etc/server.xml";
	struct server_config sc;
	long lifespan = std::numeric_limits<long>::max();
	bool run_as_daemon = true;
	int ch;

	while ((ch = getopt(argc, argv, "hvVc:d")) != EOF) {
		switch (ch) {
		case 'h':
			usage(argv[0]); // & exit
			break;
		case 'v':
		case 'V':
		case 'c':
			config_file = optarg;
			break;
		case 'd':
			run_as_daemon = false;
			break;
		case '?':
		default:
			fprintf(stderr, "Invalid option: %c\n", ch);
			usage(argv[0]); // & exit
			break;
		}
	}

	if (load_config(config_file, sc) < 0) {
		fprintf(stderr, "load config from %s failed.\n", config_file);
		exit(1);
	}

	dump_config(sc);

	if (run_as_daemon) {
		daemon(1, 1);
	}

	if (init_log(sc.log_properties_.c_str()) < 0) {
		fprintf(stderr, "init log from %s failed: %m\n", sc.log_properties_.c_str());
		exit(2);
	}

	SYSLOG_ERROR("bserver (%s) start.", sc.name_.c_str());

	EventManager emgr(4096);

	if (init_processor_list(sc, &emgr) < 0) {
		SYSLOG_ERROR("init processor list failed.");
		exit(3);
	}		

	if (init_listener_list(sc, &emgr) < 0) {
		SYSLOG_ERROR("init listener list failed.");
		exit(5);
	}

	SYSLOG_ERROR("%s start OK.", sc.name_.c_str());
	do {
		emgr.loop(lifespan);
	} while (0);
	
	SYSLOG_ERROR("bserver (%s) exit.", sc.name_.c_str());
	return 0;
}

