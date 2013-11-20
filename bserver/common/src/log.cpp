#include <log4cpp/PropertyConfigurator.hh>
#include "log.h"

log4cpp::Category* syslog()
{
	return &log4cpp::Category::getInstance("root");
}

log4cpp::Category* extlog(const char *name)
{
	return &log4cpp::Category::getInstance(name);
}

int init_log(const char *log_properties_file)
{
	try {
		log4cpp::PropertyConfigurator::configure(log_properties_file);
	} catch(log4cpp::ConfigureFailure& f) {
		fprintf(stderr, "configure(%s) failed: %s\n", log_properties_file, f.what());
		return -1;
	}

	return 0;
}

void exit_log()
{
	// nothing	
}

