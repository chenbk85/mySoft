#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define PID_FILE_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

static int lockfile(int fd)
{
	struct flock f1;

	f1.l_type = F_WRLCK;
	f1.l_start = 0;
	f1.l_whence = SEEK_SET;
	f1.l_len = 0;

	return(fcntl(fd, F_SETLK, &f1));
}

static pid_t read_pid_fd(int fd)
{
	char buf[128];
	ssize_t rlen = read(fd, buf, sizeof(buf));

	if (rlen > 0) return strtoul(buf, NULL, 0);
	return 0;
}


static int write_pid_file(const char *file)
{
	int fd = open(file, O_RDWR|O_CREAT, PID_FILE_MODE);
	char buf[128];

	if (fd < 0) return -1;

	snprintf(buf, sizeof(buf), "%lu", (unsigned long)getpid());
	write(fd, buf, strlen(buf));

	close(fd);
	return 0;
}

int check_pid_file(const char *file)
{
	pid_t pid = 0;

	int fd = open(file, O_RDWR|O_CREAT, PID_FILE_MODE);
	if (fd < 0) return -1;

	if (lockfile(fd) < 0) {
		pid = read_pid_fd(fd);
	}

	close(fd);
	return pid;
}

int create_pid_file(const char *file)
{
	pid_t pid = check_pid_file(file);
	if (pid == 0) {
		return write_pid_file(file);
	}

	return pid;
}

