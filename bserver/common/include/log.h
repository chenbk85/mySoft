#ifndef __LOG__H
#define __LOG__H

#include <log4cpp/Category.hh>
#include <signal.h>

int init_log(const char *log_properties);
void exit_log();

#define SYSLOG(l, s...) do { \
		if (syslog()->isPriorityEnabled(l)) { \
			syslog()->log((l), s); \
		} \
	} while (0)

//#define SYSLOG_EMERG(s...) SYSLOG(log4cpp::Priority::EMERG, s)
#define SYSLOG_FATAL(s...) SYSLOG(log4cpp::Priority::FATAL, s)
//#define SYSLOG_ALERT(s...) SYSLOG(log4cpp::Priority::ALERT, s)
//#define SYSLOG_CRIT(s...) SYSLOG(log4cpp::Priority::CRIT, s)
#define SYSLOG_ERROR(s...) SYSLOG(log4cpp::Priority::ERROR, s)
#define SYSLOG_WARN(s...) SYSLOG(log4cpp::Priority::WARN, s)
#define SYSLOG_INFO(s...) SYSLOG(log4cpp::Priority::INFO, s)
#define SYSLOG_DEBUG(s...) SYSLOG(log4cpp::Priority::DEBUG, s)
#ifdef UNIT_TEST
#define SYSLOG_UTEST(s...) SYSLOG_DEBUG(s)
#else /* UNIT_TEST */
#define SYSLOG_UTEST(s...) do {} while (0)
#endif /*!UNIT_TEST */

#define EXTLOG(n, l, s...) do { \
		if (extlog(n)->isPriorityEnabled(l)) { \
			extlog(n)->log((l), s); \
		} \
	} while (0)

//#define EXTLOG_EMERG(n, s...) EXTLOG(log4cpp::Priority::EMERG, n, s)
#define EXTLOG_FATAL(n, s...) EXTLOG(log4cpp::Priority::FATAL, n, s)
//#define EXTLOG_ALERT(n, s...) EXTLOG(log4cpp::Priority::ALERT, n, s)
//#define EXTLOG_CRIT(n, s...) EXTLOG(log4cpp::Priority::CRIT, n, s)
#define EXTLOG_ERROR(n, s...) EXTLOG(log4cpp::Priority::ERROR, n, s)
#define EXTLOG_WARN(n, s...) EXTLOG(log4cpp::Priority::WARN, n, s)
#define EXTLOG_INFO(n, s...) EXTLOG(log4cpp::Priority::INFO, n, s)
#define EXTLOG_DEBUG(n, s...) EXTLOG(log4cpp::Priority::DEBUG, n, s)
#ifdef UNIT_TEST
#define EXTLOG_UTEST(n, s...) EXTLOG_DEBUG(n, s)
#else /* UNIT_TEST */
#define EXTLOG_UTEST(n, s...) do {} while (0)
#endif /*!UNIT_TEST */

log4cpp::Category* syslog();
log4cpp::Category* extlog(const char *name);

__attribute__((unused))
static void sig_usr1_log_priority(int signo)
{
	struct {
		int   level;
		char *name;
	} log_level[] = {
		{ log4cpp::Priority::EMERG,  " EMERG  " },
		{ log4cpp::Priority::FATAL,  " FATAL  " },
		{ log4cpp::Priority::ALERT,  " ALERT  " },
		{ log4cpp::Priority::CRIT,   " CRIT   " },
		{ log4cpp::Priority::ERROR,  " ERROR  " },
		{ log4cpp::Priority::WARN,   " WARN   " },
		{ log4cpp::Priority::NOTICE, " NOTICE " },
		{ log4cpp::Priority::INFO,   " INFO   " },
		{ log4cpp::Priority::DEBUG,  " DEBUG  " },
		{ log4cpp::Priority::NOTSET, " NOTSET " }
	};
	if (signo == SIGUSR1) {
                static const int log_level_size = sizeof(log_level) / sizeof(log_level[0]);
                static int index = 0;
                index = (index + 1) % log_level_size;
                syslog()->setPriority(log_level[index].level);
                syslog()->fatal("===== LOG4CPP: set priority: %s =====", log_level[index].name);
	}
}

#endif /*!__LOG__H */

