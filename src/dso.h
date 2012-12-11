#ifndef DSO_H
#define DSO_H

#define OK 0
#define ERR 1

#define MAGIC_MAJOR_NUMBER 20121221
#define MAGIC_MINOR_NUMBER 0


#define STANDARD_CHANNEL_STUFF MAGIC_MAJOR_NUMBER, \
			       MAGIC_MINOR_NUMBER, \
			       __FILE__

typedef struct Channel{
	int version;
	int minor_version;
	const char *name;
	int (*handle)(char *msg);
} Channel;

void dso_load(Channel **channel, char *path, char *name);
int dso_exec(Channel *channel, char *submsg);

#endif
