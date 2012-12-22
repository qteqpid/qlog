#ifndef QSIGNAL_H
#define QSIGNAL_H

extern void initsignal(void);
extern int addsignalhandle(int signo, void (*handle)(int));
extern int blockothersignals(void);
extern int blockallsignals(void);

#endif
