#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#include "hiredis.h"
#include "server.h"
#include "qstring.h"
#include "dso.h"
#include "dict.h"
#include "mymalloc.h"
#include "qsignal.h"


static struct Qlog qlog;

const char * LOG_STR[] = {
	"DEBUG",
	"INFO",
	"WARN",
	"ERROR"
};


static unsigned int hashFunc(const void *key) {
    return dictGenHashFunction((unsigned char*)key,strlen((char*)key));
}

static void * keyDup(void *privdata, const void *key)
{
	((void)privdata);
    return qstrdup(key);
}

static int keyCompare(void *privdata, const void *key1, const void *key2)
{
	((void)privdata);
    return strcmp((char *)key1,(char *)key2) == 0;
}

static void keyDestructor(void *privdata, void *key)
{
	((void)privdata);
	if (key != NULL) free(key);
	key = NULL;
}

static struct dictType type= {
	hashFunc,         /* hash function */
	keyDup,           /* key dup */
	NULL,             /* value dup */
	keyCompare,       /* key compare */
	keyDestructor,    /* key destructor */
	NULL	          /* value destructor */
};

static void initQlogConfig(void)
{
	qlog.daemonize = 0;
	qlog.pidfile = qstrdup("/etc/qlog/qlog.pid");
	qlog.conffile = qstrdup("/etc/qlog/conf/qlog.conf");
	qlog.channeldir = qstrdup("/etc/qlog/channels/");
	qlog.logfile = NULL;
	qlog.loglevel = QLOG_LEVEL_INFO;	
	qlog.logsink = NULL;
	qlog.server = qstrdup("127.0.0.1");
	qlog.port = REDIS_DEFAULT_PORT;
	qlog.timeout_sec = 1;
	qlog.timeout_usec = 500000;
	qlog.check_interval= 3600;
	qlog.category = dictCreate(&type, NULL);
	qlog.channels = dictCreate(&type, NULL);
}

static void version(void)
{
	fprintf(stdout, "Qlog version %s\n", QLOG_VERSION);
	exit(0);
}

static void usage(void)
{
	fprintf(stderr, "Usage: ./qlog-server /path/to/qlog.conf\n");
	exit(1);
}

static void createdir(char * dir)
{
	if(access(dir,0) != 0) {
		mkdir(dir,0);
	}
}

void qlogLog(int level, const char *fmt, ...)
{
	time_t now = time(NULL);
	va_list ap;
	FILE *fp;
	char buf[64];
	char msg[QLOG_MAX_LOGMSG_LEN];	

	if (level < qlog.loglevel) return;

	//fp = (qlog.logfile == NULL) ? stdout : fopen(qlog.logfile, "a");
	//if (!fp) return;
	fp = stdout;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap); 

	if (level == QLOG_LEVEL_ERROR)
		sprintf(msg, "%s [%s:%d]", msg, __FILE__, __LINE__);

	strftime(buf, sizeof(buf), "%Y%m%d %H:%M:%S", localtime(&now));
	fprintf(fp, "[%s] [%s] %s\n", buf, LOG_STR[level], msg);
	fflush(fp);
	//if (qlog.logfile) fclose(fp);
}

static void loadQlogConfig(void)
{
	FILE *fp;
	char buf[QLOG_MAX_CONFLINE_LEN+1];
	char *line = NULL;
	char **argv = NULL;
	const char *err = NULL;
	int argc = 0, linenum = 0;

	qlogLog(QLOG_LEVEL_INFO, "Use conf_file: %s", qlog.conffile);
	if ((fp = fopen(qlog.conffile, "r")) == NULL) {
		err = "Can't open conf_file";
		goto conferr;
	} 

	while (fgets(buf, QLOG_MAX_CONFLINE_LEN+1, fp) != NULL) {
		linenum++;
		line = strim(buf);
		
		if (line[0] == '#' || line[0] == '\0') continue;
		argv = strsplit(line,'=',&argc,1);
		if (argc == 2) {
			if (strcasecmp(argv[0], "daemonize") == 0) {
				if ((qlog.daemonize = yesnotoi(argv[1])) == -1) {
					err = "value must be 'yes' or 'no'"; goto conferr;
				}
			} else if (strcasecmp(argv[0], "logsink") == 0) {
				qlog.logsink = qstrdup(argv[1]);
				createdir(qlog.logsink);
				qlog.logfile = strcat2(2, qlog.logsink, "qlog.log");
			} else if (strcasecmp(argv[0], "channeldir") == 0) {
				qlog.channeldir = qstrdup(argv[1]);
			} else if (strcasecmp(argv[0], "channel") == 0) {
				dictAdd(qlog.channels, argv[1], NULL);
			} else if (strcasecmp(argv[0], "check_interval") == 0) {
				qlog.check_interval = atoi(argv[1]);
			} else {
				err = "Unknown directive"; goto conferr;
			}
		} else {
			err = "directive must be 'key=value'"; goto conferr;
		}
		
	}
	return;

conferr:
	qlogLog(QLOG_LEVEL_ERROR, "Bad directive in conf_file[line:%d] %s", linenum, err);	
	qlogExit(15);
}

static void daemonize(void)
{
	int fd;
	qlogLog(QLOG_LEVEL_INFO, "Daemonized...");	
	if (fork() != 0) exit(0);
	setsid();
	
	if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO)
			close(fd);
	}

	if (qlog.logfile != NULL && (fd = open(qlog.logfile, O_RDWR | O_APPEND | O_CREAT, 0)) != -1) {
		dup2(fd, STDOUT_FILENO);
		if (fd > STDERR_FILENO)
			close(fd);
	}

}


void qlogExit(int sig)
{
	/* remove pid file */
    if (sig > 0)
    	remove(qlog.pidfile);

    /* free qlog allocated memory */
    free(qlog.pidfile);
    free(qlog.conffile);
    free(qlog.channeldir);
    free(qlog.server);
    dictRelease(qlog.channels);
    dictRelease(qlog.category);

    /* free mymalloc */
    myfree(0); // for string

	exit(sig);
}

void restart(void)
{
	//TODO
	qlogLog(QLOG_LEVEL_INFO, "Restart Qlog...");
}

static void signalHandler(int sig)
{
	qlogLog(QLOG_LEVEL_INFO, "Signal caught: %d", sig);
	switch(sig) {
	case SIGHUP:
		restart();
        break;
	case SIGTERM:
		qlogExit(sig);
		break;
    default:
        // do nothing
        break;
	}
}

static void setupSignalHandlers(void)
{

    //TODO: using sigaction
    initsignal();
    
	addsignalhandle(SIGHUP,signalHandler);
	addsignalhandle(SIGTERM,signalHandler);

    blockothersignals();
}

static int existPidfile(void)
{
	/* check if pid file exist */
	if ((access(qlog.pidfile, 0) != -1)) {/* pidfile exists*/
		qlogLog(QLOG_LEVEL_ERROR, "Pidfile exists. Is another qlog running?");
        return 1;
	}
    return 0;
}

static void createPidfile(void)
{
    //TODO: using open lockfile write instead
    FILE *f = fopen(qlog.pidfile, "w");
    if (f) {
        fprintf(f, "%d\n", (int)getpid());
    	fflush(f);
		fclose(f);	
	} else {
		qlogLog(QLOG_LEVEL_ERROR, "Can't create pidfile:%s", qlog.pidfile);
		exit(1);
	}
}

static void initQlog(void)
{


	qlogLog(QLOG_LEVEL_INFO, "Initializing Qlog(pid=%d)...", (int)getpid());

	/* setupSignalHandler */
	qlogLog(QLOG_LEVEL_INFO, "Setup handlers for signals");
	setupSignalHandlers();

    /* load channels */
	dictIterator *it = NULL;
	dictEntry *de = NULL;
	Channel * channel = NULL;
	
	it = dictGetIterator(qlog.channels);
	while((de = dictNext(it)) != NULL) {
		dso_load(&channel, 
			 strcat2(4, qlog.channeldir, "chnl_", (char *)dictGetEntryKey(de), ".so"), 
			 (char *)dictGetEntryKey(de));

		if (channel != NULL) {
			dictReplace(qlog.channels, dictGetEntryKey(de), channel);
		} else {
			qlogLog(QLOG_LEVEL_ERROR, "Load channel %s failed!", dictGetEntryKey(de));
			dictReleaseIterator(it);
			qlogExit(1);
		}
	}
	dictReleaseIterator(it);
	
	/* init redisContext */
	struct timeval timeout = { qlog.timeout_sec, qlog.timeout_usec }; // 1.5 seconds
	qlog.ctx = redisConnectWithTimeout(qlog.server, qlog.port, timeout);
	if (qlog.ctx->err) {
		qlogLog(QLOG_LEVEL_ERROR, "Connection error: %s", qlog.ctx->errstr);
		exit(1);
	}
	qlog.qctx = redisConnectWithTimeout(qlog.server, qlog.port, timeout);
	if (qlog.qctx->err) {
		qlogLog(QLOG_LEVEL_ERROR, "Connection error: %s", qlog.qctx->errstr);
		exit(1);
	}
}

void handleData(const char * ac)
{
	FILE *fp = NULL;
	redisReply * reply;
	char *cname = qstrdup(ac);
	char *file = strcat2(2, qlog.logsink, cname);

	//TODO: thread
	if ((fp = fopen(file, "w"))) {
			
		while ((reply = redisCommand(qlog.qctx, "lpop %s", cname))) {
			if (reply->type == REDIS_REPLY_NIL) break;
			fprintf(fp, "%s\n", reply->str);
			freeReplyObject(reply);
		}
		fflush(fp);
		fclose(fp);
		
		dictDelete(qlog.category, cname);
	} else {
		qlogLog(QLOG_LEVEL_WARN, "Open file<%s> failed", file);
		qlogExit(1);
	}
	
}

static void runQlog(void)
{
	dictIterator *it;
	dictEntry *de;
	Channel *channel;

    qlogLog(QLOG_LEVEL_INFO, "Run Qlog...");

    /* subscribe channels */
	it = dictGetIterator(qlog.channels);
	while ((de = dictNext(it)) != NULL) {
        qlogLog(QLOG_LEVEL_INFO, "Subscribe %s...", dictGetEntryKey(de));
		qlog.reply = redisCommand(qlog.ctx,"SUBSCRIBE %s", dictGetEntryKey(de));
		freeReplyObject(qlog.reply);
	}
	dictReleaseIterator(it);

    /* wait for channel request */
	while(redisGetReply(qlog.ctx,(void *)&qlog.reply) == REDIS_OK) {
		if ((de = dictFind(qlog.channels, qlog.reply->element[1]->str)) != NULL) {
            qlogLog(QLOG_LEVEL_INFO, "Trigger channel %s...", qlog.reply->element[1]->str);
			channel = (Channel *)dictGetEntryVal(de);
			dso_exec(channel, qlog.reply->element[2]->str);	
		} else {
			/* should not be here */
			qlogLog(QLOG_LEVEL_WARN, "Unknown channel: %s", qlog.reply->element[2]->str);
		}

		freeReplyObject(qlog.reply);
	}

	redisFree(qlog.ctx);
	qlogExit(0);
}

int main(int argc, char **argv) 
{
	initQlogConfig();
	if (argc == 2) {
		if (strcmp(argv[1], "-v") == 0||
				strcmp(argv[1], "--version") == 0)
			version();
		if (strcmp(argv[1], "--help") == 0||
				strcmp(argv[1], "-h") == 0)
			usage();
		strreplace(&qlog.conffile, argv[1]);
	} else if (argc == 1) {
		// no args. default conffile will be used
	} else {
		usage();
	} 
	
	loadQlogConfig();

    if (existPidfile()) qlogExit(-1);
    if (qlog.daemonize) daemonize();
	createPidfile();

	initQlog();
    runQlog();
	return 0;
}
