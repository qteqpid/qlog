#include <signal.h>
#include <stdlib.h>
#include "qsignal.h"

static sigset_t smask;

/*
 * add all signals to smaks by using sigfillset
 */
void initsignal(void)
{
    sigfillset(&smask);
}

/*
 * attach handle to signo
 * return 0 if success, otherwise return -1
 */
int addsignalhandle(int signo, void (*handle)(int)) 
{
    struct sigaction sa;
    sa.sa_handler = handle;
    sa.sa_flags = SA_NODEFER;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigdelset(&smask, signo);

    return sigaction(signo, &sa, NULL);
}

/*
 * block all signals
 * return 0 if success, otherwise return -1
 */
int blockallsignals(void)
{
    initsignal();
    return sigprocmask(SIG_BLOCK, &smask, NULL);
}

/*
 * block other signals which were not added via addsignalhandle
 * return 0 if success, otherwise return -1
 */
int blockothersignals(void) 
{
    return sigprocmask(SIG_BLOCK, &smask, NULL);
}

