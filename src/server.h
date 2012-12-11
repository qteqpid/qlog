#ifndef SERVER_H
#define SERVER_H

#include "hiredis.h"
#include "dict.h"

/* Global vars */
#define QLOG_VERSION "1.0.0"

#define QLOG_MAX_LOGMSG_LEN   1024
#define QLOG_MAX_CONFLINE_LEN 1024
#define REDIS_DEFAULT_PORT    6379

#define QLOG_LEVEL_DEBUG 0
#define QLOG_LEVEL_INFO  1
#define QLOG_LEVEL_WARN  2
#define QLOG_LEVEL_ERROR 3


struct Qlog{
	redisContext *ctx;
	redisContext *qctx;
	redisReply *reply;
	int daemonize;	
        int loglevel;
        char *logfile;
        char *pidfile;
        char *conffile;
        char *logsink;
        char *channeldir;
	char *server;
	int port;
	int timeout_sec;
	int timeout_usec;
	int check_interval;
	struct dict *category;
	struct dict *channels;
}Qlog;

extern void qlogLog(int level, const char *fmt, ...);
extern void restart(void);
extern void qlogExit(int sig);

#endif
