#include <dlfcn.h>
#include "dso.h"
#include "server.h"

void dso_load(Channel **channel, char *path, char *name)
{
	void *rv = NULL;
	void *handle = NULL;
	*channel = NULL;

	if ((handle = dlopen(path, RTLD_GLOBAL | RTLD_NOW)) == NULL) {	
		qlogLog(QLOG_LEVEL_ERROR, "Load channel %s fail: %s", name, dlerror());
		qlogExit(1);
	}
	if ((rv = dlsym(handle, name)) == NULL) {
		qlogLog(QLOG_LEVEL_ERROR, "Load channel %s fail: %s", name, dlerror());
		qlogExit(1);
	}
	*channel = (Channel *)rv;
}

int dso_exec(Channel *channel, char *submsg)
{
	if (channel->handle) {
		return channel->handle(submsg);
	}
	return ERR;
}
