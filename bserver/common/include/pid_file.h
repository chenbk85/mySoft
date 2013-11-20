#ifndef __PID_FILE__H
#define __PID_FILE__H

/* check pid file and return
 *  -1: something wrong
 *   0: not running
 * > 0: previous pid
**/
int check_pid_file(const char *file);

/* check and write the current pid to
 * pid file
**/
int create_pid_file(const char *file);

#endif /*! __PID_FILE__H */
