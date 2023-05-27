#ifndef __AESDSOCKET_H__
#define __AESDSOCKET_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT "9000"
#define MAX_BUF 1024
#define FILE_WRITE_PATH "/var/tmp/aesdsocketdata"
#define SERVER_BACK_LOG 3U

void vLogFailAndExit(char *error_string, int retval_error);
int daemonize();

#endif // __AESDSOCKET_H__